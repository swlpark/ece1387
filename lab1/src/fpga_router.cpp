#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <queue>
#include "grid_net.h"
#include "grid_cell.h"
#include "utility.h"

using namespace std;

struct NetCompByDistance {
    public :
       bool operator()(const GridNet &a, const GridNet &b) const{
          return a.getLinearDistance() < b.getLinearDistance;    
       } 
}

struct NetCompByDistance {

}

int main() {
    int                      g_size, ch_width = 0;
    //Global net-lists
    priority_queue<GridNet, vector<GridNet>, NetCompByDistance> net_heap;
    //Dikstra heap, used for Coarse-Routing
    priority_queue<GridCell, vector<GridCell>> cr_heap;
    //Heap, used to extract the lowest-cost net
    priority_queue<GridNet, vector<GridNet>> dr_heap;

    //parse standard input
    string line;
    while(getline(stdin, line)) {
        istringstream iss(line);
        if (!g_size) {
            if (!(iss >> g_size)) {
                stderr << "ERROR: Failed to parse grid size... exiting...";
                exit(1);
            }
            continue;
        } 
        if (!ch_width) {
            if (!(iss >> ch_width)) {
                stderr << "ERROR: Failed to parse channel width... exiting...";
                exit(1);
            }
            continue;
        } 

        int s_x, s_y, s_pin, t_x, t_y, t_pin;

        if (iss >> s_x >> s_y >> s_pin >> t_x >> t_y >> t_pin) {
            GridNet net(s_x, s_y, s_pin, t_x, t_y, t_pin);
            netlist.push(net);
        } else {
            stderr << "ERROR: Failed to parse a path definition... exiting...";
            exit(1);
        }
    }

    GridCell::CH_WIDTH = ch_width;
    int grid_dim = 2 * g_size + 1;

    buildFpgaGrid(grid_dim);

    return 0;
}

