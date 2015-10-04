#ifndef _FPGA_ROUTER_H_
#define _FPGA_ROUTER_H_

struct NetCompByDistance {
    public :
       bool operator() (const GridNet *a, const GridNet *b);
}

struct CellCompByCellCost {
    public :
       bool operator() (const GridCell *a, const GridCell *b);
}

//Global FPGA cell grid
extern vector<vector<GridCell>> g_fpga_grid;

//Global list of nets
extern list<GridNet> g_fpga_nets;

void doCrMazeRoute();

#endif 
