#include "main.h"

std::vector<Vertex>              cells; 
std::vector<Vertex>              fixed_cells;
std::vector<std::vector<int>>    nets; 
std::vector<double>              edge_weights;
std::vector<std::vector<double>> Q;
std::vector<double>              diag_entries;

void buildQ       (int);
void assignCellPos(std::vector<double> const &, int);
std::vector<double> computeHPWL();

bool compVertex (Vertex const& lhs, Vertex const& rhs) {
  return lhs.v_id < rhs.v_id;
}

int main(int argc, char *argv[]) {
   using namespace std;

   string   f_name;
   ifstream in_file;

   char arg;
   cout << "Starting A2 application... \n" ;
   while ((arg = getopt (argc, argv, "i:")) != -1) {
      switch (arg) {
         case 'i': 
            f_name = string(optarg);
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
   vector<double> solved_x_y = solveQ(Q, fixed_cells);
   assignCellPos(solved_x_y, Q.size());
   vector<double> hwpl_vec = computeHPWL();

   begin_graphics();
}

//**************************************************************************
//* compute bounding box HP WL of each net
//**************************************************************************
std::vector<double> computeHPWL()
{
  std::vector<double> retval;
  retval.resize(nets.size(), 0);
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
  return retval;
}

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

//Q: create Q matrix for AP, built as a list of columns
void buildQ(int f_pin_cnt)
{
   using namespace std;
   int m_dim = cells.size() - f_pin_cnt;
   Q.resize(m_dim);
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
   for (int r=0; r<m_dim; ++r)
   {
     cout << "Row " << r << ": ";
     for(int c=0; c<m_dim; ++c)
     {
         assert(Q[r][c] == Q[c][r]);

         if(!(Q[c][r] < 0))
            cout << " ";
         cout << Q[c][r] << " ";
     }
     cout << "\n";
   }
}
