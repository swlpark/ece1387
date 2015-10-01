#include <list>
#include <limit>
#include <cmath>
#include <vector>
#include "grid_net.h"

using namespace std;

//Different from LB pin definition from the input definition
enum class CellSide {SOUTH=0, EAST=1, NORTH=2, WEST=3};
enum class CellType {LOGIC_BLOCK, V_CHANNEL, H_CHANNEL, SWITCH_BOX};

//TODO: what to return for an edge?
struct CellEdge {
       int     i_pin;     //input pin
       int     o_pin;     //output pin number
};

class GridCell {
        struct CellPin {
           bool    routed;
           int     net_idx;     //idx to routed net in net_list
           int     net_ref_cnt; //incremented on every expand call made by individual CellNets
        };

        //Cell Co-ordinates on Grid
        int               m_x_pos, m_y_pos;
        CellType          m_type;
        int               m_adj_cnt;
        GridCell          *m_adj_south;
        GridCell          *m_adj_east;
        GridCell          *m_adj_north;
        GridCell          *m_adj_west;

        //TODO:need mechanism to rotate through edges on each getGrEdgesCall
        list  <GridNet*>  m_net_list;
        vector<CellPin>   m_pin_list;

        int      __calcCellCost(bool);
    public:

        //Coarse Routing Scratch
        //------------------------------
        //Dikstra back-tracking ptr
        int         m_cr_path_cost;
        GridCell   *m_cr_pred;
        bool        m_cr_reached;

        static int  s_ch_width;
        static bool s_uni_track;

        GridCell();
        GridCell(int, int) ;
        ~GridCell();
     
        int             setAdjacency  (const GridCell *, const GridCell *, const GridCell *, const GridCell *); 
        bool operator < (const GridCell&) const;

        //return "global congestion" cost of using this cell
        int           getCrCellCost();                      

        int           addCrNet     (GridNet *);             //Add a net to the cell
        int           removeCrNet  (GridNet *);             //remove a net assigned to this cell

        //CellNet will call getDrEdges on C and S cells; returns number of edges written to vector
        int           getDrEdges   (int, const GridCell *, vector<CellEdge> &); //"expand given an input pin, and target cell" 
        int           routeDrNet   (int, int) ;           //trigger update to attached nets
};

int  GridCell::s_ch_width;
bool GridCell::s_uni_track;

//TODO: default constructor
//GridCell::GridCell()
//{

//}

GridCell::GridCell(int _x_pos, int _y_pos, const GridCell * _s_ptr, const GridCell * _e_ptr, const GridCell, )
: m_x_pos(_x_pos), m_y_pos(_y_pos), m_net_list() {
        if ((_m_x_pos % 2) && (_m_y_pos % 2)) {
           //(Odd, Odd)
           m_type = CellType::LOGIC_BLOCK;
        } else if((_m_x_pos % 2) && !(_m_y_pos % 2)) {
          //(Odd, Even)
          m_type = CellType::V_CHANNEL;
        } else if(!(_m_x_pos % 2) && (_m_y_pos % 2)) {
          //(Even, Odd)
          m_type = CellType::H_CHANNEL;
        } else {
          //(Even, Even)
          m_type = CellType::SWITCH_BOX;

        }
}

/*
 * reutrn 0 if successful
 */

int GridCell::setAdjacency(const GridCell * _s_ptr, const GridCell * _e_ptr, const GridCell * _n_ptr, const GridCell * _w_ptr) const{
    m_adj_cnt = 0;
    if (_s_ptr != nullptr) {
      m_adj_south = _s_ptr;
      m_adj_cnt++;
    }
    if (_e_ptr != nullptr) {
      m_adj_east = _e_ptr;
      m_adj_cnt++;
    }
    if (_n_ptr != nullptr) {
      m_adj_north = _n_ptr;
      m_adj_cnt++;
    }
    if (_w_ptr != nullptr) {
      m_adj_east = _w_ptr;
      m_adj_cnt++;
    }
    if (m_adj_cnt < 2) {
      //note: adjacnecy should always be greater than 2
      return 1;
    } ()
    if (m_type != CellType::LOGIC_BLOCK) {
      //# of sides * channel_width = num pins
      m_pin_list.reserve(s_ch_widthi * m_adj_cnt);
    } else {
      m_pin_list.reserve(4);
    } 
  return 0;
}


bool GridCell::operator < (const GridCell & _cell) const{
     return (m_cr_path_cost < _cell.m_cr_path_cost);
}

GridCell::addNet(GridNet * _net) {

GridCell::getCrEdgeCost() {
    switch (m_type) {
       CellType::LOGIC_BLOCK :
           return numeric_limits<int>::max();
       CellType::V_CHANNEL :
           return __calcCellCost(false);
       CellType::H_CHANNEL :
           return __calcCellCost(false);
       CellType::SWITCH_BOX :
           return __calcCellCost(true);
    } 
}:

//TODO: implement linear & quadratic cost functions later
int GridCell::__calcCellCost(bool is_sbox) {
    if (is_sbox) {
       //TODO:
       //maximum fan-in for S-Box is 2*CH_WIDTH
       if (m_net_list.size() == 2 * s_ch_width) {
           return numeric_limits<int>::max();
       } else {
           return 1;
       }
    } else {
       if (m_net_list.size() == s_ch_width) {
           return numeric_limits<int>::max();
       } else {
           return 1;
       }
    }
}
