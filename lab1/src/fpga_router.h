#ifndef _FPGA_ROUTER_H_
#define _FPGA_ROUTER_H_
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <cstdlib>
#include <algorithm>
#include <iterator>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include "grid_net.h"
#include "grid_cell.h"
#include "utility.h"

class NetCompByDistance {
    public :
       bool operator() (GridNet *a, GridNet *b);
};

class CellCompByPathCost {
    public :
       bool operator() (GridCell *a, GridCell *b);
};

//Global FPGA cell grid
extern std::vector<std::vector<GridCell>> g_fpga_grid;

//Global list of nets
extern std::vector<GridNet> g_fpga_nets;
void printNetInfo(int, bool);

#endif 
