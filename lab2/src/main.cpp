#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <cstdlib>
#include <algorithm>
#include <iterator>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

int main(int argc, char *argv[]) {
   using namespace std;

   string   f_name;
   ifstream in_file;

   char c;
   cout << "Starting A2 application... \n" ;
   while ((c = getopt (argc, argv, "ui:")) != -1) {
      switch (c) {
         case 'u':
            u_uni_directional = true;
            break;
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

   //vector<vector<int>> data;

   //parse standard input
   string line;
   int net_id = 1;
   while(getline(in_file,line)) {
      istringstream iss(line);
      int cell_id;
      if (!(iss >> cell_id)) {
         cerr << "I/O Error: A input line does not start with an int... exiting...\n";
         exit(EXIT_FAILURE);
      } else {
         vector<int> line_data;
         int value;
         while(iss >> value) {
           line_data.push_back(value);
         }
         if(*(--line_data.end()) == -1) {
            if (line_data.size() > 1) {

            } 
         }
      }

   }

   if (in_file.bad()) {
      cerr << "I/O Error: There was a problem(s) with reading the file - " << f_name << "\n"; 
      exit(EXIT_FAILURE);
   }

   in_file.close();
   cout << "Okay: finished parsing the placer input file : " << f_name << "\n"; 
}
