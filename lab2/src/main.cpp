#include "main.h"
#define MIN_SWAP_TRIALS 1000

//cells, a vector of both movable and fixed I/O cells
std::vector<Vertex>                 cells; 
//nets, nets connecting cells as described in the input file
std::vector<std::vector<int>>       nets; 
//edge_weights, clique-model weights assoicated with each net
std::vector<double>                 edge_weights;
//fixed_cells, virtual pins added for recursive spreading
std::vector<Vertex>                 fixed_cells;
//virtual_pins, virtual pins added for recursive spreading
std::vector<Vertex>                 virtual_pins;
//Q, quadratic clique wire length optimization matrix for movable cells
std::vector<std::vector<double>>    Q;

//vp_idx, virtual pin id number (1 2 3 4 ...)
int vp_idx=0;

bool compVertex (Vertex const& lhs, Vertex const& rhs) {
  return lhs.v_id < rhs.v_id;
}

int main(int argc, char *argv[]) {
   using namespace std;

   string   f_name;
   ifstream in_file;

   int  opt_weight = 3;
   bool opt_swap = false;
   bool opt_spread = false;
   bool opt_recursive_spread = false;
   bool opt_verbose = false;

   char arg;
   cout << "Starting A2 application... \n" ;

   while ((arg = getopt (argc, argv, "w:i:cwsrv")) != -1) {
      switch (arg) {
         case 'i': 
            f_name = string(optarg);
            break;
         case 'c': 
            opt_swap = true;
            break;
         case 'w': 
            opt_weight = (int) strtoul(optarg, NULL, 10);
            break;
         case 's': 
            opt_spread = true;
            break;
         case 'r': 
            opt_recursive_spread = true;
            break;
         case 'v': 
            opt_verbose = true;
            break;
      }
   }

   in_file.open(f_name);
   if (!in_file.is_open()) {
      cerr << "Cannot open file - " << f_name << "; please check file exists\n";
      exit(EXIT_FAILURE);
   }

   //**************************************************************************
   //* Start of file I/O: parse input file as a stream
   //**************************************************************************
   string line;
   bool mark_io = false;
   bool sorted  = false;

   while(getline(in_file,line))
   {
      istringstream iss(line);
      int value;
      if (!(iss >> value)) {
         cerr << "I/O Error: A input line does not start with an int... exiting...\n";
         exit(EXIT_FAILURE);
      }
      else
      {
         vector<int> line_data;
         line_data.push_back(value);
         while(iss >> value) {
           line_data.push_back(value);
         }

         if (line_data.size() == 0)
            continue;

         //lines terminating with -1
         if(*(--line_data.end()) == -1)
         {
            if (line_data.size() > 1)
            {
              Vertex c; 
              c.v_id = line_data.at(0);
              cells.push_back(c);
     
              for(unsigned int i=1; i<line_data.size(); ++i)
              {
                int e = line_data.at(i);
                if (e == -1) break;
                while ((int)nets.size() < e) {
                   nets.push_back(vector<int>());
                }

                nets[e-1].push_back(c.v_id);
              }
            } else { //-1 line
              mark_io = true;
              continue;
            }
         }
         else //fixed cell line(s)
         {
            assert(mark_io);
            if (!sorted) {
               sort(cells.begin(), cells.end(), compVertex);
               sorted = true;
            }
            int cell_idx = line_data.at(0) - 1;
            assert(cell_idx+1 == cells[cell_idx].v_id);

            cells[cell_idx].fixed = true;
            cells[cell_idx].x_pos = line_data.at(1);
            cells[cell_idx].y_pos = line_data.at(2);
         }
      }
   }
   if (in_file.bad()) {
      cerr << "I/O Error: There was a problem(s) with reading the file - " << f_name << "\n"; 
      exit(EXIT_FAILURE);
   }
   in_file.close();
   cout << "Okay: finished parsing the placer input file : " << f_name << "\n"; 

   //**************************************************************************
   //* set clique model edge weights
   //**************************************************************************
   edge_weights.resize(nets.size());
   for(unsigned int i = 0; i < nets.size(); ++i) 
   {
     int edge_id = i + 1;
     vector<int> & adj_cells = nets.at(i);
     assert(adj_cells.size() >= 2);

     double clique_weight = 2.0 / adj_cells.size();
     edge_weights[i] = clique_weight;

     for (unsigned int j=0; j < adj_cells.size(); ++j)
     {
       int cell_idx = adj_cells.at(j) - 1;
       for (unsigned int k=0; k < adj_cells.size(); ++k)
         cells[cell_idx].addEdge(edge_id, &cells[adj_cells[k]-1], clique_weight);
     }
   }

   //**************************************************************************
   //* Build Q matrix and solve x, y positions of movable cells
   //**************************************************************************
   int f_pin_cnt=0;
   int m_idx=0;
   Vertex::v_map_table.resize(cells.size());

   for(unsigned int i = 0; i < cells.size(); ++i) 
   {
#ifdef _DEBUG_
     cells[i].printVertex();
#endif
     if (cells[i].fixed)
     {
        fixed_cells.push_back(cells.at(i));
        ++f_pin_cnt;
        Vertex::v_map_table[i] = -1;
        continue;
     }
     Vertex::v_map_table[i] = m_idx;
     ++m_idx;
   }

   buildQ(f_pin_cnt);
   vector<double> solved_x_y;
   vector<double> hpwl_vec;

   solved_x_y = solveQ(Q, fixed_cells, nullptr);
   assignCellPos(solved_x_y, Q.size());
   hpwl_vec = computeHPWL();

   if (opt_spread || opt_recursive_spread) //A2:Q3 & Q4 - recursive spreading
   {
      std::vector<std::tuple<int,double,double>> m_points(Q.size());
      unsigned int m_idx = 0;
      for(unsigned int i = 0; i < cells.size(); ++i) 
      {
        if (cells[i].fixed)
           continue;
        m_points[m_idx] = std::tuple<int,double,double>(cells[i].v_id, cells[i].x_pos, cells[i].y_pos);
        ++m_idx;
      }
     double overlap_ratio = recursive_spread(m_points, (double)opt_weight, 100, std::pair<double, double>(0, 0), opt_recursive_spread, opt_verbose);
#ifdef _DEBUG_
     for(unsigned int i = 0; i < virtual_pins.size(); ++i)
        virtual_pins[i].printVertex();
#endif
      hpwl_vec = computeHPWL();
      std::cout << "//------------------------------------------------------------\n";
      std::cout << "// Recursive spreading results (option -s)\n";
      std::cout << "//------------------------------------------------------------\n";
      std::cout << "Overlap ratio on 100x100 grid: " << overlap_ratio << "\n";
      std::cout << "Number of virtual pins added: " << vp_idx << "\n";
   }

   //A2:Q2, SWAP experiments
   //Note: Q array remains constant while I/O (i.e. fixed) cells' x y pos is swapped
   if (opt_swap)
   {
      double min_hpwl = std::numeric_limits<double>::max();
      double max_hpwl = std::numeric_limits<double>::min();
      double hpwl_sum = hpwl_vec.at(hpwl_vec.size() - 1);
      double hpwl_avg = hpwl_vec.at(hpwl_vec.size() - 1);
      double relative_delta = 0;

      int swap_cnt = 0;
      //perform pin location swaps until moving avg delta is less than 1%
      do
      {
        swapIOPair(&fixed_cells[0], fixed_cells.size());
        solved_x_y = solveQ(Q, fixed_cells, nullptr);
        assignCellPos(solved_x_y, Q.size());
        hpwl_vec = computeHPWL();

        if(hpwl_vec.at(hpwl_vec.size()-1) > max_hpwl)
          max_hpwl = hpwl_vec.at(hpwl_vec.size() - 1);
        if(hpwl_vec.at(hpwl_vec.size()-1) < min_hpwl)
          min_hpwl = hpwl_vec.at(hpwl_vec.size() - 1);
        ++swap_cnt;

        hpwl_sum += hpwl_vec.at(hpwl_vec.size() - 1);
        relative_delta = fabs(hpwl_avg - (hpwl_sum / (swap_cnt + 1))) / hpwl_avg;
        hpwl_avg = hpwl_sum / (swap_cnt + 1);

      } while(relative_delta > 0.000001 || swap_cnt < MIN_SWAP_TRIALS);

      unsigned int f_idx = 0;
      //update swapped cells location in the cells vector for graphis
      for(unsigned int i = 0; i < cells.size(); ++i) 
      {
        if (!cells[i].fixed)
           continue;
        cells[i].x_pos = fixed_cells.at(f_idx).x_pos;
        cells[i].y_pos = fixed_cells.at(f_idx).y_pos;
        ++f_idx;
      }
      std::cout << "//------------------------------------------------------------\n";
      std::cout << "// Q2 Experiments result (swaps performed = " << swap_cnt << ")\n";
      std::cout << "//------------------------------------------------------------\n";
      std::cout << "The Best HPWL : " << min_hpwl << "\n";
      std::cout << "The Worst HPWL : " << max_hpwl << "\n";
      std::cout << "Average HPWL : " << hpwl_avg << "\n";
   }

   displayHPWL(hpwl_vec);
   begin_graphics();
}


double spread_to_zero_bin(std::vector<std::tuple<int,double,double>> & points, double v_pin_weight, double grid_width, std::pair<double, double> grid_zero_pos, bool verbose)
{
  assert(points.size() >= 1 && points.size() < 4);

  int overlap_bins[100] = {};
  //two way spread called when we already met overlap requirement...
  computeOverlap(overlap_bins);

  int start_row, end_row, start_col, end_col; 
  start_row = (int)floor(grid_zero_pos.second / 10.0);
  end_row   = (int)floor((grid_zero_pos.second + grid_width) / 10.0);

  if (end_row == 10)
      end_row = 9; 
  assert(start_row >= 0 && start_row < 10);
  assert(end_row >= 0 && end_row < 10);

  start_col = (int)floor( grid_zero_pos.first / 10.0 );
  end_col   = (int)floor( (grid_zero_pos.first + grid_width) / 10.0);
  if (end_col == 10)
     end_col = 9;
  assert(start_col >= 0 && start_col < 10);
  assert(end_col >= 0 && end_col < 10);

  unsigned int p_idx = 0;
  for(int r = start_row; r <= end_row; r++)
  {
    for(int c = start_col; c <= end_col; c++)
    {
       if(!(p_idx < points.size()))
          break;
       int bin_idx = (r*10) + c;
       if (overlap_bins[bin_idx] == 0) {
           Vertex v_pin;
           //build a virtual pin at the center bin
           v_pin.v_id = ++vp_idx;
           v_pin.fixed = true;
           v_pin.v_pin = true;
           v_pin.x_pos = (c*10) + 5;
           v_pin.y_pos = (r*10) + 5;

           unsigned int cell_idx = std::get<0>(points.at(p_idx)) - 1;
           assert(cell_idx < cells.size() && cell_idx >= 0);
           v_pin.addEdge(vp_idx, &cells[cell_idx], v_pin_weight);

           unsigned int Q_idx = Vertex::v_map_table.at(cell_idx);
           assert(Q_idx < Q.size() && Q_idx >= 0);
           Q[Q_idx][Q_idx] += v_pin_weight;

           virtual_pins.push_back(v_pin);
           ++p_idx;
       }
    }
  }
  assertQSymmetry(false);

  //re-solve X, Y coordinates of movable cells
  std::vector<double> solved_x_y = solveQ(Q, fixed_cells, &virtual_pins);
  assignCellPos(solved_x_y, Q.size());

  int new_overlay_bin[100] = {};
  return computeOverlap(new_overlay_bin);
}

double recursive_spread(std::vector<std::tuple<int,double,double>> & points, double v_pin_weight, double grid_width, std::pair<double, double> grid_zero_pos, bool recurse, bool verbose)
{
  static bool high_bias = false;

  assert(points.size() >= 4);

  //add virtual pin weights to diagnoal entries in Q
  for(auto it = points.begin(); it != points.end(); ++it)
  {
     unsigned int idx = Vertex::v_map_table.at(std::get<0>(*it)- 1);
     assert(!cells[std::get<0>(*it)- 1].fixed);
     assert(idx < Q.size() && idx >= 0);
     Q[idx][idx] += v_pin_weight;
  }

#ifdef _DEBUG_
  std::cout << "DEBUG: after adding to diagonal entries\n";
  std::cout << "Diag Entries : ";
  for(unsigned int i = 0; i< Q.size(); ++i)
  {
     std::cout << Q[i][i] << " ";
  }
  std::cout << "\n";
#endif

  assertQSymmetry(false);

  //partition points into 4 quadrants, sorted by x_dim and then by y_dim 
  partition_quadrants(points);
  
  //marks start of cells in 4 quadrants in the partitioned points vector
  //unsigned int sw_idx=0;
  unsigned int nw_idx;
  unsigned int se_idx;
  unsigned int ne_idx;

  //adjust indices if they are not evenly divisible by 2 or 4
  if (points.size() % 2) {
    if(high_bias) {
      nw_idx = points.size() / 4;
      se_idx = points.size() / 2; 
      ne_idx = nw_idx + se_idx + 1;
      high_bias = false;
    } else {
      nw_idx = (points.size() / 4) + 1;
      se_idx = (points.size() / 2) + 1;
      ne_idx = nw_idx + se_idx;
      high_bias = true;
    }
  } else if (points.size() % 4) {
    se_idx = points.size() / 2; 
    if(high_bias) {
      nw_idx = points.size() / 4;
      ne_idx = nw_idx + se_idx;
      high_bias = false;
    } else {
      nw_idx = (points.size() / 4) + 1;
      ne_idx = nw_idx + se_idx;
      high_bias = true;
    }
  } else { //evenly divisble by both 2, 4; no adjustment needed
    nw_idx = points.size() / 4;
    se_idx = points.size() / 2; 
    ne_idx = nw_idx + se_idx;
  }

  assert(nw_idx > 0 && se_idx > nw_idx && ne_idx > se_idx);
  double h_width = grid_width / 2;
  double q_width = grid_width / 4;

  double sw_x = grid_zero_pos.first + q_width;
  double sw_y = grid_zero_pos.second + q_width;
  double nw_y = sw_y + h_width;
  double se_x = sw_x + h_width;

  //double nw_x = sw_x;
  //double se_y = sw_y;
  //double ne_x = se_x;
  //double ne_y = nw_y;

  Vertex v_pin;
  //build a virtual pin at the SW sub-quadrant
  v_pin.v_id = ++vp_idx;
  v_pin.fixed = true;
  v_pin.v_pin = true;
  v_pin.x_pos = sw_x;
  v_pin.y_pos = sw_y;
  unsigned int cell_idx;
  for(unsigned int i = 0; i < nw_idx; ++i)
  {
     cell_idx = std::get<0>(points.at(i)) - 1;
     assert(cell_idx < cells.size() && cell_idx >= 0);
     v_pin.addEdge(vp_idx, &cells[cell_idx], v_pin_weight);
  }
  virtual_pins.push_back(v_pin);
  v_pin.adj_list = std::list<Edge>();

  //build a virtual pin at the NW sub-quadrant
  v_pin.v_id = ++vp_idx;
  v_pin.y_pos = nw_y;
  for(unsigned int i = nw_idx; i < se_idx; ++i)
  {
     cell_idx = std::get<0>(points.at(i)) - 1;
     assert(cell_idx < cells.size() && cell_idx >= 0);
     v_pin.addEdge(vp_idx, &cells[cell_idx], v_pin_weight);
  }
  virtual_pins.push_back(v_pin);
  v_pin.adj_list = std::list<Edge>();

  //build a virtual pin at the SE sub-quadrant
  v_pin.v_id = ++vp_idx;
  v_pin.x_pos = se_x;
  v_pin.y_pos = sw_y;
  for(unsigned int i = se_idx; i < ne_idx; ++i)
  {
     cell_idx = std::get<0>(points.at(i)) - 1;
     assert(cell_idx < cells.size() && cell_idx >= 0);
     v_pin.addEdge(vp_idx, &cells[cell_idx], v_pin_weight);
  }
  virtual_pins.push_back(v_pin);
  v_pin.adj_list = std::list<Edge>();

  //build a virtual pin at the NE sub-quadrant
  v_pin.v_id = ++vp_idx;
  v_pin.y_pos = nw_y;
  for(unsigned int i = ne_idx; i < points.size(); ++i)
  {
     cell_idx = std::get<0>(points.at(i)) - 1;
     assert(cell_idx < cells.size() && cell_idx >= 0);
     v_pin.addEdge(vp_idx, &cells[cell_idx], v_pin_weight);
  }
  virtual_pins.push_back(v_pin);

  //re-solve X, Y coordinates of movable cells
  std::vector<double> solved_x_y = solveQ(Q, fixed_cells, &virtual_pins);
  assignCellPos(solved_x_y, Q.size());

  int overlap_bins[100] = {};
  double retval = computeOverlap(overlap_bins);

  if (verbose)
  {
    step_graphis(retval, vp_idx);
  }

  if (recurse)
  {
     //re-curse until spread condition is met
     if (retval > 0.15)
     {
       if (countEmptyBin(overlap_bins, grid_zero_pos.first, grid_zero_pos.second, h_width)) {
         std::vector<std::tuple<int,double,double>> sw_points(&points[0], &points[nw_idx]);
         if (nw_idx >= 4) {
           std::cout << "Info: RECURSE at SW corner with size: " << sw_points.size() << "\n";
           retval = recursive_spread(sw_points, pow(v_pin_weight,1.6), h_width, std::pair<double, double>(grid_zero_pos.first, grid_zero_pos.second), recurse, verbose);
         } else if( nw_idx >= 1) { 
           retval = spread_to_zero_bin(sw_points, pow(v_pin_weight,1.6), h_width, std::pair<double, double>(grid_zero_pos.first, grid_zero_pos.second), verbose);
         }
       } else {
          std::cout << "Info: stopping spreading at SW; no empty bin available; grid_width=" << grid_width << "\n";
       }
     }
     if (retval > 0.15)
     {
       if (countEmptyBin(overlap_bins, grid_zero_pos.first, grid_zero_pos.second + h_width, h_width)) {
          std::vector<std::tuple<int,double,double>> nw_points(&points[nw_idx], &points[se_idx]); if ((se_idx - nw_idx) >= 4) { std::cout << "Info: RECURSE at NW corner with size: " << nw_points.size() << "\n";
            retval = recursive_spread(nw_points, pow(v_pin_weight,1.6), h_width, std::pair<double, double>(grid_zero_pos.first, grid_zero_pos.second + h_width), recurse, verbose);
          } else if ((se_idx - nw_idx) >= 1) {
            retval = spread_to_zero_bin(nw_points, pow(v_pin_weight,1.6), h_width, std::pair<double, double>(grid_zero_pos.first, grid_zero_pos.second + h_width), verbose);
          }
       } else {
          std::cout << "Info: stopping spreading at NW; no empty bin available; grid_width=" << grid_width << "\n";
       }
     }
     if (retval > 0.15)
     {
       if (countEmptyBin(overlap_bins, grid_zero_pos.first + h_width, grid_zero_pos.second, h_width)) {
          std::vector<std::tuple<int,double,double>> se_points(&points[se_idx], &points[ne_idx]);
          if ((ne_idx - se_idx) >= 4) {
            std::cout << "Info: RECURSE at SE corner with size: " << se_points.size() << "\n";
            retval = recursive_spread(se_points, pow(v_pin_weight,1.6), h_width, std::pair<double, double>(grid_zero_pos.first + h_width, grid_zero_pos.second), recurse, verbose);
          } else if((ne_idx - se_idx) >= 1) {
            retval = spread_to_zero_bin(se_points, pow(v_pin_weight,1.6), h_width, std::pair<double, double>(grid_zero_pos.first + h_width, grid_zero_pos.second), verbose);
          }
       } else {
          std::cout << "Info: stopping spreading at SE; no empty bin available; grid_width=" << grid_width << "\n";
       }
     }
     if (retval > 0.15)
     {
       if (countEmptyBin(overlap_bins, grid_zero_pos.first + h_width, grid_zero_pos.second + h_width, h_width)) {
          std::vector<std::tuple<int,double,double>> ne_points(&points[ne_idx], &points[points.size()]);
          if ((points.size() - ne_idx) >= 4) {
            std::cout << "Info: RECURSE at NE corner with size: " << ne_points.size() << "\n";
            retval = recursive_spread(ne_points, pow(v_pin_weight,1.6), h_width, std::pair<double, double>(grid_zero_pos.first + h_width, grid_zero_pos.second + h_width), recurse, verbose);
          } else if ((points.size() - ne_idx) >= 1){
            retval = spread_to_zero_bin(ne_points, pow(v_pin_weight,1.6), h_width, std::pair<double, double>(grid_zero_pos.first + h_width, grid_zero_pos.second + h_width), verbose);
          }
       } else {
          std::cout << "Info: stopping spreading at NE; no empty bin available; grid_width=" << grid_width << "\n";
       }
     }
  }

  return retval;
}

int countEmptyBin(int * overlap_bins, double left_x, double bottom_y, double grid_width)
{
    int retval = 0;
    int start_row, end_row, start_col, end_col; 
    start_row = (int)floor(bottom_y / 10.0);
    end_row   = (int)floor((bottom_y+grid_width) / 10.0);
    if (end_row == 10)
        end_row = 9; 
    assert(start_row >= 0 && start_row < 10);
    assert(end_row >= 0 && end_row < 10);

    start_col = (int)floor( left_x / 10.0 );
    end_col   = (int)floor( (left_x + grid_width) / 10.0);
    if (end_col == 10)
       end_col = 9;
    assert(start_col >= 0 && start_col < 10);
    assert(end_col >= 0 && end_col < 10);

    for(int r = start_row; r <= end_row; r++)
    {
      for(int c = start_col; c <= end_col; c++)
      {
         int bin_idx = (r*10) + c;
         if (overlap_bins[bin_idx] == 0)
            ++retval;
      }
    }
    return retval;
}

double computeOverlap(int * overlap_bins)
{
  //calculate overlap in each 10x10 bin in a 100x100 chip die
  unsigned int row_offset = 0;
  unsigned int col_offset = 0;

  //Total number of movable cells in a chip
  int N = 0;
  for(unsigned int i = 0; i < cells.size(); ++i) 
  {
    Vertex v = cells.at(i);
    if (v.fixed)
       continue;

    row_offset = (unsigned int)floor(v.y_pos / 10.0);
    row_offset *= 10;
    assert(row_offset >= 0 && row_offset < 100 && !(row_offset % 10));

    col_offset = (unsigned int)floor(v.x_pos / 10.0);
    assert(col_offset >= 0 && col_offset < 10);

    overlap_bins[row_offset + col_offset] += 1; 
    ++N;
  }

  std::cout << "//------------------------------------------------------------\n";
  std::cout << "// Overlap Bin Matrix 10x10\n";
  std::cout << "//------------------------------------------------------------\n";
  unsigned int overlap_cnt = 0;
  for(int r = 9; r >= 0; --r) 
  {
    std::cout << "Row " << r << ": " << overlap_bins[r*10];
    for(int c = 0; c < 10; ++c) 
    {
      if(overlap_bins[r*10+c] > 2) {
         overlap_cnt += overlap_bins[r*10+c] - 2;
      }
      std::cout << " " << overlap_bins[r*10+c];
    }
    std::cout << "\n";
  }

  double overlap_ratio = (double)overlap_cnt / (double)N;

#ifdef _DEBUG_
  std::cout << "Overlap ratio = " << overlap_ratio << "\n";
#endif

  return overlap_ratio;
}

void swapIOPair(Vertex * io_cells, unsigned int f_dim)
{
  //Seed with a real random value, if available
  std::random_device rd;
  std::default_random_engine e1(rd());
  std::uniform_int_distribution<unsigned int> uniform_dist(0, f_dim-1);

  unsigned int idx_a = uniform_dist(e1);
  unsigned int idx_b = uniform_dist(e1);

  while(idx_a == idx_b) {
   idx_b = uniform_dist(e1);
  }

  //array range bound
  assert(idx_a < f_dim && idx_a >= 0 && idx_b < f_dim && idx_b >= 0);

  double tmp_x, tmp_y;
  tmp_x =  io_cells[idx_a].x_pos;
  tmp_y =  io_cells[idx_a].y_pos;

  io_cells[idx_a].x_pos = io_cells[idx_b].x_pos;
  io_cells[idx_a].y_pos = io_cells[idx_b].y_pos;

  io_cells[idx_b].x_pos = tmp_x;
  io_cells[idx_b].y_pos = tmp_y;
}

//**************************************************************************
//* compute bounding box HPWL of each net
//**************************************************************************
std::vector<double> computeHPWL()
{
  std::vector<double> retval;
  retval.resize(nets.size()+1, 0);
  //iterate over nets
  for(unsigned int n=0; n<nets.size(); ++n) 
  {
    double max_x = std::numeric_limits<double>::min();
    double min_x = std::numeric_limits<double>::max();
    double max_y = std::numeric_limits<double>::min();
    double min_y = std::numeric_limits<double>::max();

    //iterate over cells of a net; update mins, maxs
    for(unsigned int v=0; v<nets.at(n).size(); v++)
    {
       if(cells.at(nets[n][v] - 1).x_pos > max_x)
         max_x = cells.at(nets[n][v] - 1).x_pos;
       if(cells.at(nets[n][v] - 1).x_pos < min_x)
         min_x = cells.at(nets[n][v] - 1).x_pos;

       if(cells.at(nets[n][v] - 1).y_pos > max_y)
         max_y = cells.at(nets[n][v] - 1).y_pos;
       if(cells.at(nets[n][v] - 1).y_pos < min_y)
         min_y = cells.at(nets[n][v] - 1).y_pos;
    }
    retval[n] = (max_x - min_x) + (max_y - min_y);
  }

  //last hpwl_vec entry is the sum of all HPWL
  double hpwl_sum = std::accumulate(retval.begin(), retval.end(), 0.0);
  retval[nets.size()] = hpwl_sum;
  return retval;
}

void displayHPWL(std::vector<double> const & hpwl_vec)
{
  std::cout << "//------------------------------------------------------------\n";
  std::cout << "// HPWL of placed cells (cell_cnt =" << cells.size() << ", net_cnt=" << nets.size() << ")\n";
  std::cout << "//------------------------------------------------------------\n";
  for(unsigned int i=0; i < nets.size(); i=i+1)
  {
    std::cout << "Net " << i+1 << " HPWL : " << hpwl_vec.at(i) << "\n";
  }
  std::cout << "Total HPWL= " << hpwl_vec.at(nets.size()) << "\n";
}

//**************************************************************************
//* Assign x, y positions to each movable cell
//**************************************************************************
void assignCellPos(std::vector<double> const & x_y_vec, int m_dim)
{
   int v_idx = 0;
   for(unsigned int i = 0; i < cells.size(); ++i) 
   {
     if (cells[i].fixed)
        continue;
     cells[i].x_pos = x_y_vec.at(v_idx);
     cells[i].y_pos = x_y_vec.at(v_idx+m_dim);
     ++v_idx;
   }
}

//**************************************************************************
//* buildQ: create Q matrix for AP, built as a list of columns
//**************************************************************************
void buildQ(int f_pin_cnt)
{
   using namespace std;
   int m_dim = cells.size() - f_pin_cnt;
   Q.resize(m_dim);
   std::vector<double> diag_entries;
   diag_entries.resize(m_dim);

   //iterate over movable cells, generate non-diagonal entries of Q
   int c = 0;
   int cell_idx = 0;
   while(c < m_dim)
   {
      if (cells[cell_idx].fixed)
      {
         ++cell_idx;
         continue;
      }

      list<Edge>& adj_cells = cells[cell_idx].adj_list;
      Q[c].resize(m_dim, 0.0);

      //iterating over the edge list
      for(auto it = adj_cells.begin(); it != adj_cells.end(); ++it)
      {
         if ((*it).tgt->fixed)
           continue;
         int r_idx = Vertex::v_map_table.at((*it).tgt->v_id - 1); //which row in a given column to update?
         assert(r_idx < m_dim && r_idx >= 0);
         Q[c][r_idx] -= (*it).weight;
      }
      ++c;
      ++cell_idx;
   }

   //iterate over fixed cells, add to diagonal_entries
   c = 0;
   cell_idx = 0;
   while(c < f_pin_cnt)
   {
      if (!cells[cell_idx].fixed)
      {
         ++cell_idx;
         continue;
      }

      list<Edge>& adj_cells = cells[cell_idx].adj_list;
      //iterating over the edge list
      for(auto it = adj_cells.begin(); it != adj_cells.end(); ++it)
      {
         if ((*it).tgt->fixed)
           continue;
         int idx = Vertex::v_map_table.at((*it).tgt->v_id - 1);
         assert(idx < m_dim && idx >= 0);
         diag_entries[idx] += (*it).weight;
      }
      ++c;
      ++cell_idx;
   }

   //iterate over rows, add row sum to diagonal_entries
   for (int r=0; r<m_dim; ++r)
   {
     double row_sum = 0;
     for(int c=0; c<m_dim; ++c)
     {
        row_sum += fabs(Q[c][r]);
     }
     diag_entries[r] += row_sum;
     Q[r][r] = diag_entries[r];
   }

   //check that Q matrix is symmetrical
   assertQSymmetry(false);
}

void assertQSymmetry(bool print)
{
   using namespace std;
   //check that Q matrix is symmetrical
   for (unsigned int r=0; r<Q.size(); ++r)
   {
     if(print) cout << "Row " << r << ": ";
     for(unsigned int c=0; c<Q.size(); ++c)
     {
         assert(Q[r][c] == Q[c][r]);
         if(!(Q[c][r] < 0))
           if(print) cout << " ";
         if(print) cout << Q[c][r] << " ";
     }
     if(print) cout << "\n";
   }

}
