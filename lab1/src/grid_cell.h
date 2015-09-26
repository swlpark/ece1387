#include <list>
#include <limit>
#include <cmath>

using namespace std;

enum CellPinOut     {SOUTH=1, EAST=2, WEST=3, NORTH=4};
enum class CellType {LOGIC_BLOCK, V_CHANNEL, H_CHANNEL, SWITCH_BOX};

class GridCell {
        //Cell Co-ordinates on Grid
        int           m_x_pos, m_y_pos;
        CellType      m_type;

        list<GridNet> m_net_list;

        //Coarse Routing Scratch
        //------------------------------
        //Dikstra back-tracking ptr
        GridCell *m_cr_pred;

        //HEAP to return the next low cost wire;
        int      __calcCellCost(bool);

    public:
        static int  s_ch_width;
        static bool s_uni_track;

        GridCell();
        GridCell(int, int) ;
        ~GridCell();

        int          getGrCellCost();
        void         getLrEdges(int , GridCell &); //return edges jump to target cell
        void         set(int , GridCell &); //return edges jump to target cell
};

//Number of routing tracks in each channel cell
int GridCell::s_ch_width;
int GridCell::s_uni_track;

//TODO: implement 
//GridCell::GridCell()
//{

//}

GridCell::GridCell(int x_pos, int y_pos) : m_x_pos(0), m_y_pos(0) {
        if ((m_x_pos % 2) && (m_y_pos % 2)) {
                //(Odd, Odd)
                c_type = CellType::LOGIC_BLOCK;
        } else if((m_x_pos % 2) && !(m_y_posj % 2)) {
                //(Odd, Even)
                c_type = CellType::V_CHANNEL;
        } else if(!(m_x_pos % 2) && (m_y_pos % 2)) {
                //(Even, Odd)
                c_type = CellType::H_CHANNEL;
        } else {
                //(Even, Even)
                c_type = CellType::SWITCH_BOX;
        }
}

GridCell::getGrEdgeCost() {
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
GridCell::__calcCellCost(bool is_sbox) {
    if (is_sbox) {
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
