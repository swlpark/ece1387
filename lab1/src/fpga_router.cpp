#include "fpga_router.h"

//Global FPGA cell grid
std::vector<std::vector<GridCell>> g_fpga_grid;

//Global list of nets
std::vector<GridNet> g_fpga_nets;

//Nets with longer linear distance between src tgt get routed first
bool NetCompByDistance::operator() (GridNet *a, GridNet *b) {
   return a->getLineDistance() < b->getLineDistance();    
} 

//cell with the lowest total path cost get routed first
bool CellCompByPathCost::operator() (GridCell *a, GridCell *b) {
   return (a->m_cr_path_cost) > (b->m_cr_path_cost);
} 

void printNetInfo (int net_id, bool new_line) {
  int net_idx = net_id - 1;
  std::cout << " net_id = " << net_id << ", line_dist = " << g_fpga_nets.at(net_idx).getLineDistance() << "; " \
  <<" src("  << g_fpga_nets.at(net_idx).m_src_x << ", " << g_fpga_nets.at(net_idx).m_src_y << ", " \
  << g_fpga_nets.at(net_idx).m_src_p << ");"  << " tgt("  << g_fpga_nets.at(net_idx).m_tgt_x << ", " \
  << g_fpga_nets.at(net_idx).m_tgt_y << ", " << g_fpga_nets.at(net_idx).m_tgt_p << ");";
  if (new_line)
     std::cout << "\n";
}

/*
*  Main maze routing function, using Dikstra's shortest path algorithm with back-tracking
*/
bool dikstraMazeRoute (Coordinate src, Coordinate tgt, bool first_trial, int bt_track, GridNet * net)  {
    //Dikstra heap, used for Coarse-Routing
    std::priority_queue<GridCell*, std::vector<GridCell*>, CellCompByPathCost> wavefront;

    //for each MazeRoute run, current_track is constant
    int current_track;

    std::vector<int> tracks;
    std::vector<int> trials;

    //push source node to wavefront
    g_fpga_grid[src.y][src.x].m_cr_path_cost = 0;
    wavefront.push(&g_fpga_grid[src.y][src.x]);

    bool success = false;
    while (!wavefront.empty()) {
        GridCell* c = wavefront.top();
        wavefront.pop();
        c->m_cr_reached = true;

        //Check if c is the target cell;
        Coordinate tmp(c->m_x_pos, c->m_y_pos, tgt.p);
        if (tmp == tgt) { //c is matching target, backtrack and expand
           std::cout << "ROUTING INFO: Target net at (" << tgt.x << ", " << tgt.y  << ") found\n";
           while(c != nullptr) { //back track
              c->addNet(net);     //for congestion cost calculation
              net->insertNode(c); //constructing a linked list of path

              //TODO:Update CHANNEL's track, matching predecessors value (UNI-directional tracks only)
              if(GridCell:s_uni_track && c->m_cr_pred == CellType::SWITCH_BOX) {
                 //on ODD track, adjust track for SOUTH and EAST cell
                 if ((c->m_cr_pred->m_cr_track % 2)) {
                    if(c->m_adj_south == (*iter)) {
                       (*iter)->m_cr_track = c->m_cr_track - 1;
                       switched_tr = true;
                       std::cout << ", South-ward: SWITCHED TRACK TO " << (*iter)->m_cr_track << " FROM " << c->m_cr_track;
                       //current_track-=1; 
                    }
                    else if (c->m_adj_east == (*iter)) {
                       (*iter)->m_cr_track = c->m_cr_track - 1;
                       switched_tr = true;
                       std::cout << ", East-ward SWITCHED TRACK TO " << (*iter)->m_cr_track << " FROM " << c->m_cr_track;
                       //current_track-=1; 
                    }
                 } 
                 //on EVEN track, adjust track NORTH and WEST cell
                 else if (!(c->m_cr_track % 2) && c->m_adj_north == (*iter)) {
                    if(c->m_adj_north == (*iter)) {
                       (*iter)->m_cr_track = c->m_cr_track + 1;
                       switched_tr = true;
                       std::cout << ", North-ward: SWITCHED TRACK TO " << (*iter)->m_cr_track << " FROM " << c->m_cr_track;
                       //current_track+=1; 
                    }
                    else if (c->m_adj_west == (*iter)) {
                       (*iter)->m_cr_track = c->m_cr_track + 1;
                       switched_tr = true;
                       std::cout << ", West-ward: SWITCHED TRACK TO " << (*iter)->m_cr_track << " FROM " << c->m_cr_track;
                       //current_track+=1; 
                    }
                 }
              }

              c = c->m_cr_pred;
           } 
           //validate & expand
           success = net->routeGraph(src.x, src.y);
           break;
        }

        //Iterate over c's adjacent neighbors
        std::vector<GridCell*> adj_cells = c->getAdjCells(src.p);
        std::cout << "\nDEBUG : expanding C cell (" << tostring_cell_type(c) <<  ") at (" << c->m_x_pos << ", " << c->m_y_pos << "), m_cr_track=" << c->m_cr_track <<", m_cr_path_cost=" << c->m_cr_path_cost << "\n";
        std::cout << "DEBUG : adj_cells.size() = " << adj_cells.size();

        //FIRST channel case: starting cell must choose a track
        if (c->m_cr_path_cost == 0 && c->m_type == CellType::LOGIC_BLOCK && first_trial) {
           GridCell * first_channel = adj_cells.at(0);
           if(first_channel->m_type == CellType::SWITCH_BOX || first_channel->m_type == CellType::LOGIC_BLOCK) {
              std::cerr << "CRITICAL: FIRST HOP FROM LB is not a channel, C cell (" << tostring_cell_type(c) <<  ") at (" << c->m_x_pos << ", " << c->m_y_pos << ")\n";
              exit(EXIT_FAILURE);
           }

           tracks.resize(GridCell::s_ch_width);
           int num_tracks = first_channel->getTracks(&tracks[0]);
           std::cout << "INFO: got " << num_tracks << " tracks from, first channel at (" << c->m_x_pos << ", " << c->m_y_pos << ")\n";
           trials.resize(num_tracks);

           //should we fail to find a route with a chosen track, we will try other tracks later 
           for(int i = 0; i < num_tracks; ++i) {
              trials[i] = 0;
           }

           if (num_tracks > 1) {
              //Method 1: Randomly choose
              //int r_idx = std::rand() % num_tracks;
              //std::srand(std::time(0)); //use current time as rand seed
              //current_track = tracks.at(r_idx);
              //std::cout << "INFO: using " << r_idx << "-th track, from the returned list; track_num=" << current_track << "\n";
              //trials[r_idx] = 1;
              
              //Method 2: use the least referenced track
              std::vector<int> ref_array;
              for(auto i = tracks.begin(); i != tracks.end(); ++i) {
                 //push ref cnt of returned track
                 ref_array.push_back(GridCell::s_track_ref.at(*i));
              }
              auto min_it = std::min_element(ref_array.begin(), ref_array.end());
              int least_used_idx = std::distance(ref_array.begin(), min_it);
              current_track = tracks.at(least_used_idx);
              std::cout << "INFO: using " << least_used_idx << "-th track, from the returned list; track_num=" << current_track << "\n";
              trials[least_used_idx] = 1;

           } else if (num_tracks == 1) {
              current_track = tracks.at(0);
              std::cout << "INFO: using one track from the returned list; track_num=" << current_track << "\n";
           } else if (num_tracks == 0) {
              std::cout << "INFO: OUT OF available tracks, exiting ... \n";
              break;
           }
           
           //update Dikstra node fields, and continue loop
           c->m_cr_track = current_track;
           first_channel->m_cr_pred = c;
           first_channel->m_cr_path_cost = 1;
           first_channel->m_cr_track = current_track;
           wavefront.push(first_channel);
           continue;
        } else if (c->m_cr_path_cost == 0 && c->m_type == CellType::LOGIC_BLOCK && !first_trial) {
           GridCell * first_channel = adj_cells.at(0);
           //Back-tracking Dikstra run; attempt with a given track
           current_track = bt_track;
           std::cout << "INFO: Backtracking updated current track to " << current_track << "\n";
           c->m_cr_track = current_track;
           first_channel->m_cr_pred = c;
           first_channel->m_cr_path_cost = 1;
           first_channel->m_cr_track = current_track;
           wavefront.push(first_channel);
           continue;
        }

        for(auto iter=adj_cells.begin(); iter!=adj_cells.end(); ++iter) {
           bool switched_tr = false;
           int tr = 0;
           std::cout << "\n-> child: (" << tostring_cell_type((*iter)) <<  ") at (" << (*iter)->m_x_pos << ", " << (*iter)->m_y_pos << ") m_cr_track=" << (*iter)->m_cr_track  << ", cr_path_cost=";
           if ((*iter)->m_cr_path_cost == std::numeric_limits<int>::max()) std::cout << "INF";
           else std::cout << (*iter)->m_cr_path_cost;
           if ((*iter)->m_cr_reached) {
              std::cout << ", REACHED";
              continue;
           }

           //Enforce UNITRACK rule
           if (GridCell::s_uni_track) {
              if(c->m_type == CellType::V_CHANNEL) {
                 //on ODD track, skip EAST cell
                 if ((c->m_cr_track % 2) && c->m_adj_east == (*iter) ) {
                    std::cout << ", SKIPPED EAST";
                    continue; 
                 } 
                 //on EVEN track, skip WEST cell
                 else if (!(c->m_cr_track % 2) && c->m_adj_west == (*iter)) {
                    std::cout << ", SKIPPED WEST";
                    continue; 
                 }
              }
              else if(c->m_type == CellType::H_CHANNEL) {
                 //on ODD track, skip SOUTH cell
                 if ((c->m_cr_track % 2) && c->m_adj_south == (*iter) ) {
                    std::cout << ", SKIPPED SOUTH";
                    continue; 
                 } 
                 //on EVEN track, skip NORTH cell
                 else if (!(c->m_cr_track % 2) && c->m_adj_north == (*iter)) {
                    std::cout << ", SKIPPED NORTH";
                    continue; 
                 }
              } else if(c->m_type == CellType::SWITCH_BOX) {
                 //on ODD track, adjust track for SOUTH and EAST cell
                 if ((c->m_cr_track % 2)) {
                    if(c->m_adj_south == (*iter)) {
                       (*iter)->m_cr_track = c->m_cr_track - 1;
                       switched_tr = true;
                       std::cout << ", South-ward: SWITCHED TRACK TO " << (*iter)->m_cr_track << " FROM " << c->m_cr_track;
                       //current_track-=1; 
                    }
                    else if (c->m_adj_east == (*iter)) {
                       (*iter)->m_cr_track = c->m_cr_track - 1;
                       switched_tr = true;
                       std::cout << ", East-ward SWITCHED TRACK TO " << (*iter)->m_cr_track << " FROM " << c->m_cr_track;
                       //current_track-=1; 
                    }
                 } 
                 //on EVEN track, adjust track NORTH and WEST cell
                 else if (!(c->m_cr_track % 2) && c->m_adj_north == (*iter)) {
                    if(c->m_adj_north == (*iter)) {
                       (*iter)->m_cr_track = c->m_cr_track + 1;
                       switched_tr = true;
                       std::cout << ", North-ward: SWITCHED TRACK TO " << (*iter)->m_cr_track << " FROM " << c->m_cr_track;
                       //current_track+=1; 
                    }
                    else if (c->m_adj_west == (*iter)) {
                       (*iter)->m_cr_track = c->m_cr_track + 1;
                       switched_tr = true;
                       std::cout << ", West-ward: SWITCHED TRACK TO " << (*iter)->m_cr_track << " FROM " << c->m_cr_track;
                       //current_track+=1; 
                    }
                 }
              }
           }

           if (switched_tr)
              tr = (*iter)->m_cr_track;
           else
              tr = c->m_cr_track;

           if (!GridCell::s_uni_track && tr != current_track)
              std::cerr << "\nERROR: CURRENT_TRACK =" << current_track <<"; DOES NOT MATCH tr track=" << tr;

           //either non-target LB, or no available pin to route with current_track
           if ((*iter)->getCellCost(tgt.x, tgt.y, tgt.p, tr, c) == std::numeric_limits<int>::max()) {
              ////CHECK_BOX
              //if (c->m_type != CellType::SWITCH_BOX)  current_track = c->m_cr_track;
              std::cout << " , NOT ADJACENT TO PIN: " << tgt.p;
              continue;
           }

           int tmp_dist = c->m_cr_path_cost + (*iter)->getCellCost(tgt.x, tgt.y, tgt.p, tr, c);
           if (tmp_dist < (*iter)->m_cr_path_cost) {
              (*iter)->m_cr_pred = c;
              (*iter)->m_cr_path_cost = tmp_dist;
               
              //(*iter)->m_cr_track = current_track;
              if (!switched_tr) (*iter)->m_cr_track = c->m_cr_track;
              //   std::cout << " UPDATE pred=" << "(" << tostring_cell_type((*iter)) <<  ") at (" << (*iter)->m_x_pos << ", " << (*iter)->m_y_pos << "), path_cost=" << tmp_dist;
           }
           wavefront.push(*iter);
        } //adjacent cells loop
        std::cout << "\n";
     } //end wavefront  loop

     //Clean up grid for next Dikstra run
     for (auto r_it = g_fpga_grid.begin(); r_it != g_fpga_grid.end(); ++r_it) {
        for (auto c_it = r_it->begin(); c_it != r_it->end(); ++c_it) {
           c_it->m_cr_path_cost = std::numeric_limits<int>::max();
           c_it->m_cr_pred = nullptr;
           c_it->m_cr_reached = false;
           //NEW: CHECK
           c_it->m_cr_track = 0;
        }
     }
     //wavefront= priority_queue <GridCell*, vector<GridCell*>, CellCompByPathCost> (); //reset heap
     int cnt=1;
     if(!success && first_trial) {
       std::cout << "NET_ID= "<< net->m_net_id << ";First run with track=" << current_track << " didn't work, so we are backtracking with other tracks.\n";
       std::cout << "DEBUG: tracks.size()=" << tracks.size() << "\n";
       for(int t=0; t < trials.size(); t++) {
          if (trials.at(t) == 1) continue;

          if(dikstraMazeRoute(src, tgt, false, tracks.at(t) , net))
            return true;
          else {
            std::cout << "\nNET_ID= "<< net->m_net_id << ";Backtracking " <<  cnt << "-th time, and it didin't work with current_track=" << current_track << "\n\n";
            cnt++;
          }
          if (t > GridCell::s_ch_width) {
            std::cerr << "Backtracked more than there is available tracks... CRITICAL ERROR...\n";
            return false;
          }
       }
     }

    //if successful, increment track reference count
    if (success)
      GridCell::s_track_ref[current_track] += 1;

     return success;
}

int main(int argc, char *argv[]) {
   using namespace std;
   //Command Line option parsing:
   //1) unidirectional -u, bidirectional tracks -b
   //2) channel width -W 
   //3) gui
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

   //Nets to route
   priority_queue<GridNet*, vector<GridNet*>, NetCompByDistance> net_heap;
   list<GridNet*> failed_nets;

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
         if (s_x < 0 || s_y < 0 || s_p < 0 || t_x < 0 || t_y < 0 || t_p < 0) {
            cout << "I/O: Reached EOF line\n";
            break;
         }
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
   GridCell::s_track_ref.resize(ch_width, 0);
   GridCell::s_uni_track = u_uni_directional;

   int grid_dim = 2 * g_size + 1;

   build_FPGA_grid(g_fpga_grid, grid_dim);
   print_FPGA_grid(g_fpga_grid);

   //add nets to heap to be used for coarse-routing
   for(auto l_it = g_fpga_nets.begin(); l_it != g_fpga_nets.end(); ++l_it) {
      net_heap.push(&(*l_it));
   }

   int fail_cnt = 0;
   int tracks_used = 0;
   //NOTE: net is solid object here..
   while(!net_heap.empty()) {
     GridNet* net = net_heap.top();
     net_heap.pop();
     Coordinate src = net->getSrcCoordinate();
     Coordinate tgt = net->getTgtCoordinate();

     //Start of Dikstra's algorithm for coarse routing
     cout << "\n//-----------------------------------------------------------//\n";
     cout << "//Start routing";
     printNetInfo(net->m_net_id, true);
     cout << "//-----------------------------------------------------------//\n";
 
     if (!dikstraMazeRoute(src, tgt, true, 0, net)) {
        cout << "ROUTING: couldn't expand to the destination cell during maze routing phase\n";
        cout << "NetID: " << net->m_net_id << "\n";
        ++fail_cnt;
        failed_nets.push_back(net);
        continue;
     } else { //Successfully routed the net
       //Tracks Used = Total Path Levels - 2 (i.e. LB pins are not tracks)
       tracks_used += net->m_graph.size() - 2;
     }
   }//end net_heap loop

   begin_graphics();

   cout << "\n//-----------------------------------------------------------//\n";
   cout << "// Summary\n";
   cout << "// Channel Width = " << GridCell::s_ch_width << "\n";
   cout << "// ";
   if (GridCell::s_uni_track) cout << "Uni-directional Tracks\n";
   else                       cout << "Bi-directional Tracks\n";
   cout << "// Number of nets to route = " << g_fpga_nets.size() << "\n";
   cout << "//-----------------------------------------------------------//\n";
   cout << "Number of tracks used: " << tracks_used << "\n";
   cout << "Number of nets that failed to route: " << fail_cnt << "\n\n";

   for(auto i = failed_nets.begin(); i != failed_nets.end(); ++i) {
     printNetInfo ((*i)->m_net_id, true);
   }
   return EXIT_SUCCESS;
}

