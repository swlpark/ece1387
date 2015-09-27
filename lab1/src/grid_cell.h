#include <list>
#include <limit>
#include <cmath>
#include <vector>

using namespace std;

enum CellSide {SOUTH=1, EAST=2, WEST=3, NORTH=4};
enum class CellType {LOGIC_BLOCK, V_CHANNEL, H_CHANNEL, SWITCH_BOX};

class GridCell {
        struct CellTrack {

        };

        //Cell Co-ordinates on Grid
        int            m_x_pos, m_y_pos;
        CellType       m_type;

        list<GridNet*> m_net_list;
        vector<int>    m_net_list;

        //TODO:Data struct to manage wires on grid cell
        //TODO:HEAP/function to return the next low cost wire;
        //Think about how to represent
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
     
        bool operator < (const GridCell&) const;

        int           getCrCellCost();                 //return "congestion" cost of using this cell
        void          getDrEdges   (int , GridCell &); //return edges jump to target cell
        int           addCrNet     (GridNet *);        //Add a net to the cell
        int           removeCrNet  (GridNet *);        //
        void          assignWire   (int , GridCell &); //trigger update to attached nets
};

int  GridCell::s_ch_width;
bool GridCell::s_uni_track;

//TODO: default 
//GridCell::GridCell()
//{

//}

GridCell::GridCell(int _x_pos, int _y_pos) : m_x_pos(_x_pos), m_y_pos(_y_pos), m_net_list() {
        if ((_m_x_pos % 2) && (_m_y_pos % 2)) {
                //(Odd, Odd)
                c_type = CellType::LOGIC_BLOCK;
        } else if((_m_x_pos % 2) && !(_m_y_pos % 2)) {
                //(Odd, Even)
                c_type = CellType::V_CHANNEL;
        } else if(!(_m_x_pos % 2) && (_m_y_pos % 2)) {
                //(Even, Odd)
                c_type = CellType::H_CHANNEL;
        } else {
                //(Even, Even)
                c_type = CellType::SWITCH_BOX;
        }
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
}

//TODO: implement linear & quadratic cost functions later
int GridCell::__calcCellCost(bool is_sbox) {
    if (is_sbox) {
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
