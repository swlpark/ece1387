enum class CellType {LOGIC_BLOCK, V_CHANNEL, H_CHANNEL, SWITCH_BOX};

class GridCell {
        //Cell Co-ordinates on Grid
        int m_x_pos, m_y_pos;
        CellType m_type;

        int m_net_cnt;

        //Global Routing Scratch
        //------------------------------
        //Dikstra path cost
        int gr_cost;
        //Dikstra back-tracking ptr
        GridCell *gr_pred;

        //Global Routing
        int lr_cost;

        //HEAP to return the next low cost;

    public:
        static int s_ch_width;
        static bool ;


        GridCell();
        GridCell(int, int) ;
        ~GridCell();

        unsigned int getGrEdgeCost();
        getLrEdges(GridCell &, GridCell &);
}

//Number of routing tracks in each channel cell
int GridCell::s_ch_width;

GridCell::GridCell() : m_x_pos(0), m_x_pos(0), gr_pred(nullptr), gr_cost(0)
{
  c_type = CellType::SWITCH_BOX;
}

GridCell::GridCell(int x_pos, int y_pos) : x_pos(0), y_pos(0), gr_cost(0) {
    if ((x_pos % 2) && (y_pos % 2)) {
        //(Odd, Odd)
        c_type = CellType::LOGIC_BLOCK;
    } else if((x_pos % 2) && !(y_posj % 2)) {
        //(Odd, Even)
        c_type = CellType::H_CHANNEL;
    } else if(!(x_pos % 2) && (y_pos % 2)) {
        //(Even, Odd)
        c_type = CellType::V_CHANNEL;
    } else {
        //(Even, Even)
        c_type = CellType::SWITCH_BOX;
    }
}


GridCell::getGrEdgeCost() {
  switch(m_type) {
    
  }
}

