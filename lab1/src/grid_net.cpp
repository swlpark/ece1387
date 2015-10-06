#include "grid_net.h"

int GridNet::s_branch_num = 4;

GridNet::GridNet() : m_graph(), o_pins() {
   m_net_id = 0;
   m_src_x = 0;
   m_src_y = 0;
   m_src_p = 0;

   m_tgt_x = 0;
   m_tgt_y = 0;
   m_tgt_p = 0;

   m_line_dist = 0;
}

GridNet::GridNet(int _id, int _s_x, int _s_y, int _s_p, int _t_x, int _t_y, int _t_p) : m_graph(), o_pins() {
   m_net_id = _id;

   m_src_x = _s_x;
   m_src_y = _s_y;
   m_src_p = _s_p;

   m_tgt_x = _t_x;
   m_tgt_y = _t_y;
   m_tgt_p = _t_p;

   m_line_dist = sqrt((pow(abs(m_tgt_x - m_src_x), 2) + pow(abs(m_tgt_y - m_src_y), 2)));
}

//delete call is recursive
GridNet::~GridNet() {
}

int GridNet::getLineDistance(){
   return m_line_dist;
}

Coordinate GridNet::getSrcCoordinate(){
   Coordinate retval(m_src_x, m_src_y, m_src_p);
   return retval;
}

Coordinate GridNet::getTgtCoordinate(){
   Coordinate retval(m_tgt_x, m_tgt_y, m_tgt_p);
   return retval;
}

int GridNet::connectPins() {
   if (m_graph.size() < 3) {
      std::cerr << "GridNet Error: must have +3 nodes on a coarse graph path\n";
      return EXIT_FAILURE;
   }
   o_pins.resize(m_graph.size());
   std::srand(std::time(0)); //use current time as random seed

   GridCell * lh_cell = nullptr; //next-hop look-ahead cell
   GridCell * parent = nullptr;
   int        parent_pin = -1;
   int        lv_cnt = 0;

   for (auto it = m_graph.begin(); it != m_graph.end(); ++it) {
      if ((*it)-> m_type == CellType::V_CHANNEL || (*it)->m_type == CellType::H_CHANNEL ) {
         lh_cell =  *(std::next(it, 1));

         //have to choose a pin on a first run
         if (parent->m_type == CellType::LOGIC_BLOCK) {
            std::vector<int> exp_pins;
            if ((*it)->getTrackBundle(s_branch_num, lh_cell, exp_pins) > 0) {
              //choose one of the available pins randomly
              int r_idx = std::rand() % exp_pins.size();
              o_pins.push_back(exp_pins.at(r_idx)); 
            } else { //ROUTING FAIL
               std::cout << "GridNet Info: not enough routing resource(s) on the first channel, @LEVEL=" << lv_cnt << "\n";
               return EXIT_FAILURE;
            }
         } else {//expanding path by one hop
           //mapping parent's o_pin to look-ahead's input pin
           int lh_i_pin = matchAdjacentPin(parent_pin , parent, (*it));
           if (lh_i_pin >= 0) {
              int lh_o_pin;
              lh_o_pin =(*it)->getOutputPin(lh_i_pin, m_tgt_p, lh_cell);
              if (lh_o_pin >= 0) {
                 parent_pin = lh_o_pin;
                 o_pins.push_back(parent_pin); 
              } else {
                 std::cout << "GridNet Info: not enough routing resource(s) on a middle channel, @LEVEL=" << lv_cnt <<"\n";
                 return EXIT_FAILURE;
              }
           } else {
              std::cerr << "GridNet Error: could not find a matching pin..., @LEVEL=" << lv_cnt <<"\n";
              return EXIT_FAILURE;
           }
         } 
      } else if ((*it)-> m_type == CellType::SWITCH_BOX) {
         lh_cell =  *(std::next(it, 1));
         //mapping parent's o_pin to look-ahead's input pin
         int lh_i_pin = matchAdjacentPin(parent_pin , parent, (*it));
         if (lh_i_pin >= 0) {
            int lh_o_pin;
            lh_o_pin =(*it)->getOutputPin(lh_i_pin, m_tgt_p, lh_cell);
            if (lh_o_pin >= 0) {
               parent_pin = lh_o_pin;
               o_pins.push_back(parent_pin); 
            } else {
               std::cout << "GridNet Info: not enough routing resource(s) on a middle channel, @LEVEL=" << lv_cnt <<"\n";
               return EXIT_FAILURE;
            }
         } else {
            std::cerr << "GridNet Error: could not find a matching pin..., @LEVEL=" << lv_cnt <<"\n";
            return EXIT_FAILURE;
         }
      } else if ((*it)-> m_type == CellType::LOGIC_BLOCK) {
         if (it == m_graph.begin()) {//first
           o_pins.push_back(m_src_p);
         } else if ((it != m_graph.end()) && (it == --m_graph.end()))  { //last
           o_pins.push_back(-1); //-1 = end
         } else {
            std::cerr << "GridNet Error: LB in the middle of graph\n";
            return EXIT_FAILURE;
         }
      }
      parent = (*it);
      //std::cout << "DEBUG: m_graph.size()=" << m_graph.size() << ", at level- " << lv_cnt  << "\n";
      lv_cnt += 1;
   } 
   return EXIT_SUCCESS;
}

void GridNet::insertNode(GridCell * node) {
   m_graph.push_front(node);
}

//check if back-tracked source cell matches the graph, and route the cell with real pins
bool GridNet::routeGraph(int src_x, int src_y) {
  bool success = false;
  if ((m_graph.front()->m_x_pos == src_x) && (m_graph.front()->m_y_pos == src_y)) {
     if(connectPins() == EXIT_FAILURE) {
        std::cout << "NET ROUTING: failed to expand the following net.\n";
        std::cout << "NetID: " << m_net_id << "\n";
     } else {
        success = true;
        std::cout << "NET ROUTING SUCCESS: following net is coarse-routed and expanded\n";
        std::cout << "NetID: " << m_net_id << "\n";
     }
     //break;
  } else {
    std::cerr << "NET ROUTING ERROR: cell src does not match expected coordinate when back-tracked\n";
    std::cerr << "NetID: " << m_net_id << "\n";
    std::cerr << "src("  << src_x << ", " << src_y << "); front(" << m_graph.front()->m_x_pos << \
    ", " << m_graph.front()->m_y_pos << ")\n";
  }
  printGraph();

  //If successful, tag the pin on the cell
  if (success) {
     int idx = 0;
     for(auto it = m_graph.begin(); it != m_graph.end(); ++it) {
        (*it)->burnPin(o_pins[idx]);
        idx += 1;
     }
  }
  return success;
}

void GridNet::printGraph() {
   if (m_graph.size() > 0) {
     int cnt = 0;
     for(auto it = m_graph.begin(); it != m_graph.end(); ++it) {
        std::cout << "Level-" << cnt++ << " : " << tostring_cell_type(*it) << "(" << (*it)->m_x_pos << ", " \
        << (*it)->m_y_pos << ") \n";
     }
   } else {
     std::cout << "GridNet: printGraph() called with zero graph\n";
   }
}
