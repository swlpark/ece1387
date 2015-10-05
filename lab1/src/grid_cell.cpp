#include "grid_cell.h"

int  GridCell::s_ch_width;
bool GridCell::s_uni_track;
int  GridCell::_last_pin_idx = 0;

GridCell::GridCell() : m_x_pos(0), m_y_pos(0), m_net_list(), m_pin_list() {
   m_adj_cnt    = 0;
   m_adj_south = nullptr;
   m_adj_east  = nullptr;
   m_adj_north = nullptr;
   m_adj_west  = nullptr;

   m_cr_path_cost = std::numeric_limits<int>::max();
   m_cr_pred      = nullptr;
   m_cr_reached   = false;

   m_type = CellType::LOGIC_BLOCK;
}

GridCell::GridCell(int _x_pos, int _y_pos) : m_x_pos(_x_pos), m_y_pos(_y_pos), m_net_list(), m_pin_list() {
   m_adj_cnt    = 0;
   m_adj_south = nullptr;
   m_adj_east  = nullptr;
   m_adj_north = nullptr;
   m_adj_west  = nullptr;

   m_cr_path_cost = std::numeric_limits<int>::max();
   m_cr_pred      = nullptr;
   m_cr_reached   = false;

   if ((m_x_pos % 2) && (m_y_pos % 2)) {
      //(Odd, Odd)
      m_type = CellType::LOGIC_BLOCK;
   } else if((m_x_pos % 2) && !(m_y_pos % 2)) {
      //(Odd, Even)
      m_type = CellType::V_CHANNEL;
   } else if(!(m_x_pos % 2) && (m_y_pos % 2)) {
      //(Even, Odd)
      m_type = CellType::H_CHANNEL;
   } else {
      //(Even, Even)
      m_type = CellType::SWITCH_BOX;
   }
}

GridCell::~GridCell() {

}

/*
 * reutrn 0 if successful
 */
int GridCell::setAdjacency(GridCell * _s_ptr, GridCell * _e_ptr, GridCell * _n_ptr, GridCell * _w_ptr){
   m_adj_cnt = 0;
   if (_s_ptr != nullptr) {
      m_adj_south = _s_ptr;
      //adjacent LB does not increase a channel's pin count
      if (! (_s_ptr->m_type == CellType::LOGIC_BLOCK) ) m_adj_cnt++;
   }
   if (_e_ptr != nullptr) {
      m_adj_east = _e_ptr;
      if (! (_e_ptr->m_type == CellType::LOGIC_BLOCK) ) m_adj_cnt++;
   }
   if (_n_ptr != nullptr) {
      m_adj_north = _n_ptr;
      if (! (_n_ptr->m_type == CellType::LOGIC_BLOCK) ) m_adj_cnt++;
   }
   if (_w_ptr != nullptr) {
      m_adj_west = _w_ptr;
      if (! (_w_ptr->m_type == CellType::LOGIC_BLOCK) ) m_adj_cnt++;
   }
   if (m_adj_cnt < 2) {
      std::cerr << "adjacnecy should always be greater or equal to 2\n";
      return EXIT_FAILURE;
   }

   int pin_cnt;
   if (m_type != CellType::LOGIC_BLOCK) {
      //# of sides * channel_width = num pins
      pin_cnt = s_ch_width * m_adj_cnt;
      m_pin_list.reserve(pin_cnt);
   } else { //4 pins for LB
      pin_cnt = 4; 
   } 
   m_pin_list.reserve(pin_cnt);
   for (int i = 0; i < pin_cnt; ++i ) {
          m_pin_list.push_back(CellPin());
          m_pin_list[i].routed = false;
          m_pin_list[i].net_ref_cnt = 0;
   }
   return EXIT_SUCCESS;
}

bool GridCell::operator < (const GridCell & _cell) const{
   return (m_cr_path_cost < _cell.m_cr_path_cost);
}

int GridCell::addCrNet(GridNet* _net) {
   auto iter = std::find(m_net_list.begin(), m_net_list.end(), _net);
   if (iter != m_net_list.end()) {
      m_net_list.push_back(_net);
      return EXIT_SUCCESS;
   } 
   return EXIT_FAILURE;
}

//TODO: finish checking if _net exists
void GridCell::removeCrNet(GridNet* _net) {
   m_net_list.remove(_net);
}

//TODO: optimization opportunity in what to push to vector
std::vector<GridCell*> GridCell::getCrAdjCells(int src_lb_pin) {
   std::vector<GridCell*> ptr_vec;
   if (m_type == CellType::LOGIC_BLOCK) {
     switch(src_lb_pin) {
       case SOUTH:
         ptr_vec.push_back(m_adj_south);
         break;
       case EAST:
         ptr_vec.push_back(m_adj_east);
         break;
       case NORTH:
         ptr_vec.push_back(m_adj_north);
         break;
       case WEST:
         ptr_vec.push_back(m_adj_west);
         break;
     }
   } else {
     if (m_adj_south != nullptr) {
        ptr_vec.push_back(m_adj_south);
     }
     if (m_adj_east != nullptr) {
        ptr_vec.push_back(m_adj_east);
     }
     if (m_adj_north != nullptr) {
        ptr_vec.push_back(m_adj_north);
     }
     if (m_adj_west != nullptr) {
        ptr_vec.push_back(m_adj_west);
     }
   }
   return ptr_vec;
}

int GridCell::getCrCellCost(int tgt_lb_pin, const GridCell * src_cell) {
   switch (m_type) {
      case CellType::LOGIC_BLOCK : //LB reachable with a given target pin
         switch(tgt_lb_pin) {
           case SOUTH:
             if (m_adj_south == src_cell)
               return 1;
             else
               return std::numeric_limits<int>::max();
           case EAST:
             if (m_adj_east == src_cell)
               return 1;
             else
               return std::numeric_limits<int>::max();
           case NORTH:
             if (m_adj_north == src_cell)
               return 1;
             else
               return std::numeric_limits<int>::max();
           case WEST:
             if (m_adj_west == src_cell)
               return 1;
             else
               return std::numeric_limits<int>::max();
         }
      case CellType::V_CHANNEL :
         return __calcCellCost(false);
      case CellType::H_CHANNEL :
         return __calcCellCost(false);
      case CellType::SWITCH_BOX :
         return __calcCellCost(true);
   } 
}

/*  TODO: implement uni-directional support
 *  expand on LB=>Channel
 */
int GridCell::getTrackBundle (int req_edges, const GridCell * tgt_cell, std::vector<int> & edges) {
   //edges simply are the output pins of this cell
   int edge_cnt = 0;
   int tgt_side; 

   if (m_type == CellType::H_CHANNEL) {
      if (m_adj_south == tgt_cell) {
        tgt_side = 0; 
      } else if (m_adj_north == tgt_cell) {
        tgt_side = s_ch_width; 
      } else {
         std::cerr << "ERROR: getTrackBundle called with target that is not adjacent. Cell: (" \
         << m_x_pos << ", " << m_y_pos << ");\n";
         return EXPAND_FAIL;
      }
   } else if (m_type == CellType::V_CHANNEL) {
      if (m_adj_east == tgt_cell) {
        tgt_side = 0; 
      } else if (m_adj_west == tgt_cell) {
        tgt_side = s_ch_width; 
      } else {
         std::cerr << "ERROR: getTrackBundle called with target that is not adjacent. Cell: (" \
         << m_x_pos << ", " << m_y_pos << ");\n";
         return EXPAND_FAIL;
      }

   } else {
      std::cerr << "ERROR: getTrackBundle called on a cell that is not a channel ( " \
      << m_x_pos << ", " << m_y_pos << ");\n";
      return EXPAND_FAIL;
   }

   //TODO:need mechanism to rotate through edges on each getGrEdgesCall
   edges.reserve(req_edges);
   for (int i = _last_pin_idx; i < s_ch_width; ++i) {
      if (edge_cnt >= req_edges) {
         break;
      } else if (!m_pin_list[(i + tgt_side)].routed) {
        ++edge_cnt;
        edges.push_back((i + tgt_side));
        m_pin_list[i].net_ref_cnt++;
        m_pin_list[(i + tgt_side)].net_ref_cnt++;
      }
   }

   _last_pin_idx %= s_ch_width;
   return edge_cnt;
}

/* TODO: implement uni-directional support
* NOTE: caller make sure that src pin is not on the same side as the target side
*/
int GridCell::getOutputPin (int src_pin, int lb_tgt_pin, const GridCell * tgt_cell) {
   //src track number on each side
   int track_idx = src_pin % s_ch_width;
   int tgt_pin = 0; 

   if (m_type == CellType::H_CHANNEL) {
      if (tgt_cell->m_type == CellType::LOGIC_BLOCK) {
         std::cout << "Dr Routing Info: Reached Logic block, returning LB_PIN= " << lb_tgt_pin << "\n";
         return lb_tgt_pin;
      } else {
         if (m_adj_south == tgt_cell) {
            tgt_pin = track_idx; 
         } else if (m_adj_north == tgt_cell) {
            tgt_pin = s_ch_width + track_idx; 
         } else {
            std::cerr << "getOutput pin is called with target that is not adjacent, src(" \
            << m_x_pos << ", " << m_y_pos << ")\n";
            return EXPAND_FAIL;
         }
      }
   } else if (m_type == CellType::V_CHANNEL) {
      if (tgt_cell->m_type == CellType::LOGIC_BLOCK) {
         std::cout << "Dr Routing Info: Reached Logic block, returning LB_PIN= " << lb_tgt_pin << "\n";
         return lb_tgt_pin;
      } else {
         if (m_adj_east == tgt_cell) {
            tgt_pin = track_idx; 
         } else if (m_adj_west == tgt_cell) {
            tgt_pin = s_ch_width + track_idx; 
         } else {
            std::cerr << "getOutput pin is called with target that is not adjacent, src(" \
            << m_x_pos << ", " << m_y_pos << ")\n";
            return EXPAND_FAIL;
         }
      }
   }  else if (m_type == CellType::SWITCH_BOX) {
      if (m_adj_south == tgt_cell) {
         tgt_pin = track_idx; 
      } else if (m_adj_east == tgt_cell) {
         tgt_pin = track_idx; 
         if (m_adj_south != nullptr) tgt_pin += s_ch_width;
      } else if (m_adj_north == tgt_cell) {
         tgt_pin = track_idx; 
         if (m_adj_south != nullptr) tgt_pin += s_ch_width;
         if (m_adj_east != nullptr)  tgt_pin += s_ch_width;

      } else if (m_adj_west == tgt_cell) {
         tgt_pin = track_idx; 
         if (m_adj_south != nullptr) tgt_pin += s_ch_width;
         if (m_adj_east != nullptr)  tgt_pin += s_ch_width;
         if (m_adj_north != nullptr) tgt_pin += s_ch_width;
      } else {
         std::cerr << "getOutput pin is called with target that is not adjacent, src(" \
         << m_x_pos << ", " << m_y_pos << ")\n";
         return EXPAND_FAIL;
      }
   } 

   if (m_pin_list[tgt_pin].routed) {
      return EXPAND_FAIL;
   }
   m_pin_list[src_pin].net_ref_cnt++;
   m_pin_list[tgt_pin].net_ref_cnt++;
   return tgt_pin;
}

//TODO: implement linear & quadratic cost functions later
int GridCell::__calcCellCost(bool is_sbox) {
   if (is_sbox) {
      //maximum fan-in for S-Box is (adjacency * s_ch_width / 2)
      if (m_pin_list.size() == (m_adj_cnt * s_ch_width / 2)) {
         return std::numeric_limits<int>::max();
      } else {
         return 1;
      }
   } else {
      if (m_pin_list.size() == s_ch_width) {
         return std::numeric_limits<int>::max();
      } else {
         return 1;
      }
   }
}
