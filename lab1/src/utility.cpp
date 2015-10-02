#include <vector>
#include "grid_net.h"
#include "grid_cell.h"

void setFpgaGrid (vector<vector<GridCell>> &grid, grid_dim) {
    grid.reserve(grid_dim);

    for (int i=0; i < grid_dim; ++i) {
        grid[i].reserve(grid_dim);
        for (int j=0; j< grid_dim; ++j) { 
            GridCell cell(i, j); 
            grid[i].push_back(cell);
        }
    }

    for (int i=0; i < grid_dim; ++i) {
        for (int j=0; j < grid_dim; ++j) { 
           if(i % 2) { //odd row
             if (j % 2) {//odd col
               //LBs...
               grid[i][j].setAdjacency(&grid[i-1][j], &grid[i][j+1],
                                            &grid[i+1][j], &grid[i][j-1]);
             } else { //even col
               //HCs...
               grid[i][j].setAdjacency(&grid[i-1][j], nullptr,
                                            &grid[i+1][j], nullptr);
             }
           } else { //even row
             if (j % 2) { //odd col
               //VCs...
               grid[i][j].setAdjacency(nullptr, &grid[i][j+1],
                                            nullptr, &grid[i][j-1]);
             } else { //even col
               //SBs...
               if (i == 0) { //on bottom row
                 if (j == 0) { //leftmost col
                   //SW corner
                   grid[i][j].setAdjacency(nullptr, &grid[i][j+1],
                                                &grid[i+1][j], nullptr);
                 } else if (j == grid_dim-1) { //rightmost col
                   //SE corner
                   grid[i][j].setAdjacency(nullptr, nullptr,
                                                &grid[i+1][j], &grid[i][j-1]);

                 } else { //south, middle cols
                   grid[i][j].setAdjacency(nullptr, &grid[i][j+1],
                                                &grid[i+1][j], &grid[i][j-1]);
                 }
               } else if (i==grid_dim-1) { //on top row
                 if (j == 0) { //leftmost col
                   //NW corner
                   grid[i][j].setAdjacency(&grid[i-1][j], &grid[i][j+1],
                                                  nullptr, nullptr);
                 } else if (j == grid_dim-1) { //rightmost col
                   //NE corner
                   grid[i][j].setAdjacency(&grid[i-1][j], nullptr,
                                                  nullptr, &grid[i][j-1]);

                 } else { //north, middle cols
                     grid[i][j].setAdjacency(&grid[i-1][j], &grid[i][j+1],
                                                  nullptr, &grid[i][j-1]);
                 }
               } else { //middle rows
                   if (j == 0) { //leftmost col
                     grid[i][j].setAdjacency(&grid[i-1][j], &grid[i][j+1],
                                                  &grid[i+1][j], nullptr);
                   } else if (j == grid_dim-1) { //rightmost col
                     grid[i][j].setAdjacency(&grid[i-1][j], nullptr,
                                                  &grid[i+1][j], &grid[i][j-1]);
                   } else { //middle cols
                     grid[i][j].setAdjacency(&grid[i-1][j], &grid[i][j+1],
                                                  &grid[i+1][j], &grid[i][j-1]);
                   }
               } //end middle rows
             } //end even col
           } //end even row
        } //end col loop
    } //end row loop
}


/*
* Return matching pin number on the target Cell
*/
int matchAdjPin (int, GridCell & p_owner, GridCell * target) {
  

}
