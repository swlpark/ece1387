#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <queue>

#include <unistd.h>

#include "grid_net.h"
#include "grid_cell.h"
#include "utility.h"

using namespace std;

struct NetCompByDistance {
    public :
       bool operator() (const GridNet &a, const GridNet &b) {
          return a.getLineDistance() < b.getLineDistance();    
       } 
}

struct CellCompByCellCost {
    public :
       bool operator() (const GridCell *a, const GridCell *b) {
          return a->getCrCellCost() < b->getCrCellCost();    
       } 
}

//Global net-lists
vector<vector<GridCell>> s_fpga_grid;

//Dikstra heap, used for Coarse-Routing
priority_queue<GridCell*, vector<GridCell*>, CellCompByCellCost> cr_heap;

void doCrMazeRoute() {

}

int main(int argc, char *argv[]) {
    int g_size, ch_width = 0;

    //TODO: 

    //Nets to route
    priority_queue<GridNet, vector<GridNet>, NetCompByDistance> s_net_heap;

    //Command Line option parsing:
    //1) unidirectional -u, bidirectional tracks -d
    //2) channel width -W 
    //2) gui

    //while ((c = getopt (argc, argv, "u:d:W")) != -1) {
    //    switch (c) {
    //    case 'i':
    //        iterations = strtoul(optarg, NULL, 10);
    //        break;
    //    case 't':
    //       num_threads = strtoul(optarg, NULL, 10);
    //       if (num_threads == 0) {
    //           printf("%s: option requires an argument > 0 -- 't'\n", argv[0]);
    //                    return EXIT_FAILURE;
    //        }
    //       break;
    //    default:
    //        return EXIT_FAILURE;
    //    }
    //}

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

        int s_x, s_y, s_p, t_x, t_y, t_p;

        if (iss >> s_y >> s_x >> s_p >> t_y >> t_x >> t_p) {
            GridNet net((2*s_x + 1), (2*s_y + 1), s_p, (2*t_x + 1), (2*t_y + 1), t_p);
            s_net_heap.push(net);
        } else {
            stderr << "ERROR: Failed to parse a path definition... exiting...";
            exit(1);
        }
    }

    //TODO: use command parameters
    GridCell::s_ch_width  = ch_width;
    GridCell::s_uni_track = false;

    int grid_dim = 2 * g_size + 1;

    buildFpgaGrid(fpga_grid, grid_dim);

    //NOTE: net is solid object here..

    while(!s_net_heap.empty()) {
      GridNet net = s_net_heap.pop();
      Coordinate src = net.getSrcCoordinate();
      Coordinate tgt = net.getTgtCoordinate();

      //Start of Dikstra's Algorithm for Coarse Routing
      fpga_grid[src.x][src.y].m_cr_path_cost = 0;
      cr_heap.push(&fpga_grid[src.x][src.y]);

      while (!cr_heap.empty()) {
        GridCell* c = cr_heap.pop();
        c->m_cr_reached = true;

        //Check if c is the target cell;
        Coordinate tmp(c->m_x_pos, c->m_y_pos, tgt.p);
        if (tmp == tgt) {

          while(c != nullptr) {
            c->addCrNet(&net); 
            m_cr_path_cost.push_front(c);
            c = c->m_cr_pred;
          } 
          //TODO: validate if c is now pointing to the source cell
          
        }

        //iterate over 
        vector<GridCell*> adj_cells = c->getCrAdjCells();
        for(auto iter=adj_cells.begin(); iter!=adj_cells.end(); ++iter ) {
          int tmp_dist = c->m_cr_path_cost + (*iter)->getCrCellCost();

          if (tmp_dist < iter->m_cr_path_cost) {
             (*iter)->m_cr_pred = c;
             (*iter)->m_cr_path_cost = tmp_dist;
          }
          cr_heap.push(*iter);
        }
      } //end cr_heap while loop

      //TODO: Clean up
      while() {}

      break;

    }
    return 0;
}

