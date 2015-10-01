#include <vector>
#include "grid_net.h"
#include "grid_cell.h"

void buildFpgaGrid (vector<vector<GridCell>> &fpga_grid, grid_dim) {
    fpga_grid.reserve(grid_dim);

    for (int i=0; i < grid_dim; ++i) {
        fpga_grid[i].reserve(grid_dim);
        for (int j=0; j< grid_dim; ++j) { 
            GridCell cell(i, j); 
            fpga_grid[i].push_back(cell);
        }
    }

    for (int i=0; i < grid_dim; ++i) {
        for (int j=0; j < grid_dim; ++j) { 
           if(i % 2) { //odd row
             if (j % 2) {//odd col
               //LBs...
               fpga_grid[i][j].setAdjacency(&fpga_grid[i-1][j], &fpga_grid[i][j+1],
                                            &fpga_grid[i+1][j], &fpga_grid[i][j-1]);
             } else { //even col
               //HCs...
               fpga_grid[i][j].setAdjacency(&fpga_grid[i-1][j], nullptr,
                                            &fpga_grid[i+1][j], nullptr);
             }
           } else { //even row
             if (j % 2) { //odd col
               //VCs...
               fpga_grid[i][j].setAdjacency(nullptr, &fpga_grid[i][j+1],
                                            nullptr, &fpga_grid[i][j-1]);
             } else { //even col
               //SBs...
               if (i == 0) { //on bottom row
                 if (j == 0) { //leftmost col
                   //SW corner
                   fpga_grid[i][j].setAdjacency(nullptr, &fpga_grid[i][j+1],
                                                &fpga_grid[i+1][j], nullptr);
                 } else if (j == grid_dim-1) { //rightmost col
                   //SE corner
                   fpga_grid[i][j].setAdjacency(nullptr, nullptr,
                                                &fpga_grid[i+1][j], &fpga_grid[i][j-1]);

                 } else { //south, middle cols
                   fpga_grid[i][j].setAdjacency(nullptr, &fpga_grid[i][j+1],
                                                &fpga_grid[i+1][j], &fpga_grid[i][j-1]);
                 }
               } else if (i==grid_dim-1) { //on top row
                 if (j == 0) { //leftmost col
                   //NW corner
                   fpga_grid[i][j].setAdjacency(&fpga_grid[i-1][j], &fpga_grid[i][j+1],
                                                  nullptr, nullptr);
                 } else if (j == grid_dim-1) { //rightmost col
                   //NE corner
                   fpga_grid[i][j].setAdjacency(&fpga_grid[i-1][j], nullptr,
                                                  nullptr, &fpga_grid[i][j-1]);

                 } else { //north, middle cols
                     fpga_grid[i][j].setAdjacency(&fpga_grid[i-1][j], &fpga_grid[i][j+1],
                                                  nullptr, &fpga_grid[i][j-1]);
                 }
               } else { //middle rows
                   if (j == 0) { //leftmost col
                     fpga_grid[i][j].setAdjacency(&fpga_grid[i-1][j], &fpga_grid[i][j+1],
                                                  &fpga_grid[i+1][j], nullptr);
                   } else if (j == grid_dim-1) { //rightmost col
                     fpga_grid[i][j].setAdjacency(&fpga_grid[i-1][j], nullptr,
                                                  &fpga_grid[i+1][j], &fpga_grid[i][j-1]);
                   } else { //middle cols
                     fpga_grid[i][j].setAdjacency(&fpga_grid[i-1][j], &fpga_grid[i][j+1],
                                                  &fpga_grid[i+1][j], &fpga_grid[i][j-1]);
                   }
               } //end middle rows
             } //end even col
           } //end even row
        } //end col loop
    } //end row loop
}
