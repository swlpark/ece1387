#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include "fpga_cell.h"

using namespace std;

int main() {
    string line;
    int g_size, ch_width = 0;
    vector<vector<GridCell>> fpga_grid;

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

        if (!(iss >> s_x >> s_y >> s_pin >> t_x >> t_y >> t_pin)) {
            stderr << "ERROR: Failed to parse a path definition... exiting...";
            exit(1);
        }

    }

    
    GridCell::CH_WIDTH = ch_width;

    int grid_dim = 2 * g_size + 1;
    fpga_grid.reserve(grid_dim);

    for (int i; i < grid_dim; ++i) {
        fpga_grid[i].reserve(grid_dim);
        for (int j; j< grid_dim; ++i) { 
            //fpga_grid[i].push_back   (i, j);
            //fpga_grid[i].emplace_back(i, j);
        }
    }
    return 0;

}

