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
   //bool u_gui             = false;
   bool u_uni_directional = false;
   int  u_width           = 8;

   string f_name;
   ifstream in_file;

   char c;
   cout << "Starting the FPGA router... \n" ;
   while ((c = getopt (argc, argv, "ui:W:")) != -1) {
      switch (c) {
         case 'u':
            u_uni_directional = true;
            break;
         case 'i': 
            f_name = string(optarg);
               break;
         case 'W':
            u_width = (int) strtoul(optarg, NULL, 10);
            if (u_width <= 0 || u_width % 2) {
               cout << "Must provide a non-zero even number for W\n" ;
               exit(EXIT_FAILURE);
            }
            break;
      }
   }
  
   in_file.open(f_name);
   if (!in_file.is_open()) {
         cerr << "Cannot open file - " << f_name << "; please check file exists\n";
         exit(EXIT_FAILURE);
   }

   int g_size = 0;
   int ch_width = 0;

   //Dikstra heap, used for Coarse-Routing
   priority_queue<GridCell*, vector<GridCell*>, CellCompByPathCost> s_cr_heap;
   //Nets to route
   priority_queue<GridNet*, vector<GridNet*>, NetCompByDistance> s_net_heap;

   //parse standard input
   string line;
   int net_id = 1;
   while(getline(in_file,line)) {
      istringstream iss(line);
      if (!g_size) {
         if (!(iss >> g_size)) {
            cerr << "I/O Error: Failed to parse grid size... exiting...\n";
            exit(EXIT_FAILURE);
         }
         continue;
      } 
      if (!ch_width) {
         if (!(iss >> ch_width)) {
            cerr << "I/O Error: Failed to parse channel width... exiting...\n";
            exit(EXIT_FAILURE);
         }
         continue;
      } 

      int s_x, s_y, s_p, t_x, t_y, t_p;
      //remember to 
      if (!(iss >> s_x >> s_y >> s_p >> t_x >> t_y >> t_p)) {
         cerr << "I/O Error: Failed to parse a path definition... exiting...\n";
         cerr << "Line: "  << line << "\n";
         exit(EXIT_FAILURE);
      } else {
         cout << "I/O netid=" << net_id << " ;" << s_x << " " << s_y << " " << s_p << " " << t_x << " " << t_y << " " << t_p << "\n";
         GridNet net(net_id, (2*s_x + 1), (2*s_y + 1), (s_p - 1), (2*t_x + 1), (2*t_y + 1), (t_p - 1));
         g_fpga_nets.push_back(net);
         ++net_id;
      }
   }

   if (in_file.bad()) {
      cerr << "I/O Error: There was a problem(s) with reading the file - " << f_name << "\n"; 
      exit(EXIT_FAILURE);
   }
   in_file.close();
   cout << "Okay: finished parsing the router input file : " << f_name << "\n"; 
   cout << "Grid Size (by LB): " << g_size << "\n"; 
   cout << "Channel width : " << ch_width << "\n"; 
   cout << "Number of nets : " << g_fpga_nets.size() << "\n"; 

   //set input parameters
   GridCell::s_ch_width  = ch_width;
   GridCell::s_uni_track = u_uni_directional;

   int grid_dim = 2 * g_size + 1;

   build_FPGA_grid(g_fpga_grid, grid_dim);
   print_FPGA_grid(g_fpga_grid);

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
      g_fpga_grid[src.y][src.x].m_cr_path_cost = 0;
      s_cr_heap.push(&g_fpga_grid[src.y][src.x]);
      cout << "\nROUTING INFO: Routing net_id = " << net->m_net_id << ", line_dist = " << net->getLineDistance() << "; " <<" src( "  << src.x << ", " << src.y << ", " << src.p << ");" \
      << " tgt("  << tgt.x << ", " << tgt.y << ", " << tgt.p << "); \n\n";

      while (!s_cr_heap.empty()) {
         GridCell* c = s_cr_heap.top();
         s_cr_heap.pop();
         c->m_cr_reached = true;

         //Check if c is the target cell;
         Coordinate tmp(c->m_x_pos, c->m_y_pos, tgt.p);
         if (tmp == tgt) { //c is matching target, backtrack and expand
            cout << "ROUTING INFO: Target net at (" << tgt.x << ", " << tgt.y  << ") found\n";
            while(c != nullptr) { //back_track
               c->addNet(net); 
               net->insertNode(c);
               c = c->m_cr_pred;
            } 
            //validate & expand
            success = net->routeGraph(src.x, src.y);
            break;
         }

         //Iterate over c's adjacent neighbors
         vector<GridCell*> adj_cells = c->getCrAdjCells(src.p);
         cout << "DEBUG : expanding C cell (" << tostring_cell_type(c) <<  ") at (" << c->m_x_pos << ", " << c->m_y_pos << ")\n";
         //cout << "DEBUG GR: C adj_cnt =" << c->m_adj_cnt << "\n";
         //cout << "DEBUG GR: C adj_south =" << c->m_adj_south << "\n";
         //cout << "DEBUG GR: C adj_east =" << c->m_adj_east << "\n";
         //cout << "DEBUG GR: C adj_north =" << c->m_adj_north << "\n";
         //cout << "DEBUG GR: C adj_west =" << c->m_adj_west << "\n";
         //cout << "DEBUG GR: adj_cells.begin() value  = " << *(adj_cells.begin()) << "\n";
         cout << "DEBUG : adj_cells.size() = " << adj_cells.size() << "\n";

         for(auto iter=adj_cells.begin(); iter!=adj_cells.end(); ++iter ) {
            cout << "-> child: (" << tostring_cell_type((*iter)) <<  ") at (" << (*iter)->m_x_pos << ", " << (*iter)->m_y_pos << ")";
            if ((*iter)->m_cr_reached) {
               cout << " REACHED\n";
               continue;
            }
            if ((*iter)->getCrCellCost(tgt.x, tgt.y, tgt.p, c) == numeric_limits<int>::max()) {
               cout << "NOT ADJACENT TO PIN: " << tgt.p << "\n";
               continue;
            }
            int tmp_dist = c->m_cr_path_cost + (*iter)->getCrCellCost(tgt.x, tgt.y, tgt.p, c);
            if (tmp_dist < (*iter)->m_cr_path_cost) {
               (*iter)->m_cr_pred = c;
               (*iter)->m_cr_path_cost = tmp_dist;
               cout << " UPDATE pred=" << "(" << tostring_cell_type((*iter)) <<  ") at (" << (*iter)->m_x_pos << ", " << (*iter)->m_y_pos << "), path_cost=" << tmp_dist;
            }
            cout << "\n";
            s_cr_heap.push(*iter);
         }
      } //end cr_heap while loop

      if (!success) {
          cout << "CR ROUTING: could'nt expand to the destination cell during CR routing \n";
          cout << "NetID: " << net->m_net_id << "\n";
          break;
      }
   
      //Clean up grid for next Dikstra run
      cout << "CR ROUTING: CLEANUP START\n";
      for (auto r_it = g_fpga_grid.begin(); r_it != g_fpga_grid.end(); ++r_it) {
         for (auto c_it = r_it->begin(); c_it != r_it->end(); ++c_it) {
            c_it->m_cr_path_cost = numeric_limits<int>::max();
            c_it->m_cr_pred = nullptr;
            c_it->m_cr_reached = false;
         }
      }
      s_cr_heap= priority_queue <GridCell*, vector<GridCell*>, CellCompByPathCost> (); //reset heap
      cout << "CR ROUTING: CLEANUP END\n";
   }//end s_net_heap loop

   return EXIT_SUCCESS;
}
