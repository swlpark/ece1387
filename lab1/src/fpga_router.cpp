#include "fpga_router.h"

//Global FPGA cell grid
std::vector<std::vector<GridCell>> g_fpga_grid;

//Global list of nets
std::list<GridNet> g_fpga_nets;

bool NetCompByDistance::operator() (GridNet *a, GridNet *b) {
   return a->getLineDistance() < b->getLineDistance();    
} 

bool CellCompByPathCost::operator() (GridCell *a, GridCell *b) {
   return (a->m_cr_path_cost) < (b->m_cr_path_cost);
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
   cout << "Starting the FPGA router... \n" ;

   while ((c = getopt (argc, argv, "uhW:")) != -1) {
      switch (c) {
         case 'u':
            u_uni_directional = true;
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
   priority_queue<GridCell*, vector<GridCell*>, CellCompByPathCost> s_cr_heap;
   //Nets to route
   priority_queue<GridNet*, vector<GridNet*>, NetCompByDistance> s_net_heap;

   //parse standard input
   string line;
   while(getline(cin, line)) {
      istringstream iss(line);
      if (!g_size) {
         if (!(iss >> g_size)) {
            cerr << "ERROR: Failed to parse grid size... exiting...";
            exit(EXIT_FAILURE);
         }
         continue;
      } 
      if (!ch_width) {
         if (!(iss >> ch_width)) {
            cerr << "ERROR: Failed to parse channel width... exiting...";
            exit(EXIT_FAILURE);
         }
         continue;
      } 

      int net_id = 1;
      int s_x, s_y, s_p, t_x, t_y, t_p;

      //remember to 
      if (iss >> s_y >> s_x >> s_p >> t_y >> t_x >> t_p) {
         GridNet net(net_id, (2*s_x + 1), (2*s_y + 1), (s_p - 1), (2*t_x + 1), (2*t_y + 1), (t_p - 1));
         g_fpga_nets.push_back(net);
         ++net_id;
      } else {
         cerr << "ERROR: Failed to parse a path definition... exiting...";
         exit(EXIT_FAILURE);
      }
   }

   //TODO: use command parameters
   GridCell::s_ch_width  = ch_width;
   GridCell::s_uni_track = false;

   int grid_dim = 2 * g_size + 1;

   build_FPGA_grid(g_fpga_grid, grid_dim);

   //add nets to Dikstra heap to be used for coarse-routing
   for(auto l_it = g_fpga_nets.begin(); l_it != g_fpga_nets.end(); ++l_it) {
      s_net_heap.push(&(*l_it));
   }

   //NOTE: net is solid object here..
	bool success = false;
   while(!s_net_heap.empty()) {
      GridNet* net = s_net_heap.top();
      s_net_heap.pop();
      Coordinate src = net->getSrcCoordinate();
      Coordinate tgt = net->getTgtCoordinate();

      //Start of Dikstra's algorithm for coarse routing
      g_fpga_grid[src.x][src.y].m_cr_path_cost = 0;
      s_cr_heap.push(&g_fpga_grid[src.x][src.y]);

      while (!s_cr_heap.empty()) {
         GridCell* c = s_cr_heap.top();
         s_cr_heap.pop();
         c->m_cr_reached = true;

         //Check if c is the target cell;
         Coordinate tmp(c->m_x_pos, c->m_y_pos, tgt.p);
         if (tmp == tgt) {
            while(c != nullptr) { //back_track
               c->addCrNet(net); 
               net->m_cr_graph.push_front(c);
               c = c->m_cr_pred;
            } 
            //validate if c is now pointing to the source cell
            if (c->m_x_pos == src.x && c->m_y_pos == src.y) {
               if(net->generateDrTree() == EXIT_FAILURE) {
                  cout << "DR ROUTING: failed to expand the following net.\n";
                  cout << "NetID: " << net->m_net_id << "\n";
               } else {
	              success = true;
               }
               break;
            } else {
              cerr << "CR ROUTING ERROR: cell src does not match expected coordinate when back-tracked\n";
              cerr << "NetID: " << net->m_net_id << "\n";
              exit(EXIT_FAILURE);
            }
         }

         //Iterate over c's adjacent neighbors
         vector<GridCell*> adj_cells = c->getCrAdjCells(src.p);
         for(auto iter=adj_cells.begin(); iter!=adj_cells.end(); ++iter ) {
            int tmp_dist = c->m_cr_path_cost + (*iter)->getCrCellCost(tgt.p, c);
            if (tmp_dist < (*iter)->m_cr_path_cost && !((*iter)->m_cr_reached)) {
               (*iter)->m_cr_pred = c;
               (*iter)->m_cr_path_cost = tmp_dist;
            }
            s_cr_heap.push(*iter);
         }
      } //end cr_heap while loop

      if (!success) {
          cout << "CR ROUTING: could'nt reach the destination cell during CR routing \n";
          cout << "NetID: " << net->m_net_id << "\n";
          break;
      }
   
      //Clean up grid for next Dikstra run
      for (auto r_it = g_fpga_grid.begin(); r_it != g_fpga_grid.end(); ++r_it) {
         for (auto c_it = r_it->begin(); c_it != r_it->end(); ++c_it) {
            c_it->m_cr_path_cost = numeric_limits<int>::max();
            c_it->m_cr_pred = nullptr;
            c_it->m_cr_reached = false;
         }
      }
   }//end s_net_heap loop

   return EXIT_SUCCESS;
}

