#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <algorithm>
#include <iterator>
#include <unistd.h>
#include <cstdlib>
#include <cassert>
#include <ctime>
#include "graph.h"

bool compVertex (Vertex const& lhs, Vertex const& rhs) {
  return lhs.v_id < rhs.v_id;
}

int main(int argc, char *argv[]) {
   using namespace std;

   string   f_name;
   ifstream in_file;

   char c;
   cout << "Starting A2 application... \n" ;
   while ((c = getopt (argc, argv, "i:")) != -1) {
      switch (c) {
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

   vector<Vertex>        cells; 
   vector<vector<int>>   edges; 
   vector<double> edge_weights;

   //parse input file as a stream
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
                while ((int)edges.size() < e) {
                   edges.push_back(vector<int>());
                }

                edges[e-1].push_back(c.v_id);
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

   //set clique model edge weights
   edge_weights.resize(edges.size());
   for(unsigned int i = 0; i < edges.size(); ++i) 
   {
     int edge_id = i + 1;
     vector<int> & adj_cells = edges.at(i);
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

   for(auto it = cells.begin(); it != cells.end(); ++it) 
   {
     (*it).printVertex();
   }
}
