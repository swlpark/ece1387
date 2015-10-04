#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <cstdlib>
#include <unistd.h>

#include "grid_net.h"
#include "grid_cell.h"
#include "utility.h"

//Global FPGA cell grid
vector<vector<GridCell>> g_fpga_grid;

//Global list of nets
list<GridNet> g_fpga_nets;

bool NetCompByDistance::operator() (const GridNet *a, const GridNet *b) {
   return a->getLineDistance() < b->getLineDistance();    
} 

bool CellCompByCellCost::operator() (const GridCell *a, const GridCell *b) {
   return a->getCrCellCost() < b->getCrCellCost();    
} 


int main(int argc, char *argv[]) {
   using namespace std;

   //Command Line option parsing:
   //1) unidirectional -u, bidirectional tracks -b
   //2) channel width -W 
   //3) gui
   bool u_uni_directional = false;
   bool u_gui             = false;
   int  u_width           = 8;

   char c;
   while ((c = getopt (argc, argv, "uhW:")) != -1) {
      switch (c) {
         case 'u':
            uni_directional = true;
            break;
         case 'h': //TODO: help menu
            cout << "\n HELP MENU" ;
            break;
         case 'W':
            u_width = (int) strtoul(optarg, NULL, 10);
            if (u_width <= 0 || u_width % 2) {
               cout << "Must provide a non-zero even number for W\n" ;
               return EXIT_FAILURE;
            }
         default:
            cout << "Running the FPGA router with default option... \n" ;
      }
   }

   int g_size, ch_width = 0;

   //Dikstra heap, used for Coarse-Routing
   priority_queue<GridCell*, vector<GridCell*>, CellCompByCellCost>  s_cr_heap;
   //Nets to route
   priority_queue<GridNet*, vector<GridNet*>, NetCompByDistance> s_net_heap;

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
         g_fpga_nets.push_back(net);
      } else {
         stderr << "ERROR: Failed to parse a path definition... exiting...";
         exit(1);
      }
   }

   //TODO: use command parameters
   GridCell::s_ch_width  = ch_width;
   GridCell::s_uni_track = false;

   int grid_dim = 2 * g_size + 1;

   buildFpgaGrid(g_fpga_grid, grid_dim);

   //TODO:
   s_net_heap.push(net);

   //NOTE: net is solid object here..
   while(!s_net_heap.empty()) {
      GridNet net = s_net_heap.pop();
      Coordinate src = net.getSrcCoordinate();
      Coordinate tgt = net.getTgtCoordinate();

      //Start of Dikstra's algorithm for coarse routing
      g_fpga_grid[src.x][src.y].m_cr_path_cost = 0;
      s_cr_heap.push(&g_fpga_grid[src.x][src.y]);

      while (!s_cr_heap.empty()) {
         GridCell* c = s_cr_heap.pop();
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

         //Iterate over c's adjacent neighbors
         vector<GridCell*> adj_cells = c->getCrAdjCells();
         for(auto iter=adj_cells.begin(); iter!=adj_cells.end(); ++iter ) {
            int tmp_dist = c->m_cr_path_cost + (*iter)->getCrCellCost();

            if (tmp_dist < (*iter)->m_cr_path_cost && !((*iter)->m_cr_reached)) {
               (*iter)->m_cr_pred = c;
               (*iter)->m_cr_path_cost = tmp_dist;
            }
            s_cr_heap.push(*iter);
         }
      } //end cr_heap while loop

      //Clean up grid for next Dikstra run
      for (auto r_it = g_fpga_grid.begin(); r_it != g_fpga_grid.end(); ++r_it) {
         for (auto c_it = r_it->begin(); c_it != r_it->end(); ++c_it) {
            c_it->m_cr_path_cost = numeric_limits<int>::max();
            c_it->m_cr_pred = nullptr;
            c_it->m_cr_reached = false;
         }
      }
      break;

   }//end s_net_heap loop
   return 0;
}

