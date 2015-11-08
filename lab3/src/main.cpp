#include "main.h"

//cells, a vector of both movable and fixed I/O cells
std::vector<Vertex>                 vertices; 
//nets, nets connecting cells as described in the input file
std::vector<std::vector<int>>       nets; 

//edge_weights, clique-model weights assoicated with each net
std::vector<double>                 edge_weights;

int main(int argc, char *argv[]) {
   using namespace std;

   string   f_name;
   ifstream in_file;

   char arg;
   cout << "Starting A3 application... \n" ;

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
            cout << "Invalid file input\n";
         }
      }
   }
   if (in_file.bad()) {
      cerr << "I/O Error: There was a problem(s) with reading the file - " << f_name << "\n"; 
      exit(EXIT_FAILURE);
   }
   in_file.close();
   cout << "Okay: finished parsing the input file : " << f_name << "\n"; 

   begin_graphics();
}


