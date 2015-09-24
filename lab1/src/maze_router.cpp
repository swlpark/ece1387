#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include "fpga_cell.h"

using namespace std;

class fpga_cell {
  enum CellType {LB, VC, HC, SB};
  ~MazeCell() {}
  public:
}

int main() {
  string line;
  int g_size, ch_width = 0;
  vector<vector<fpga_cell>>
  
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

    int s_x, s_y, s_pin, t_x, t_y, t_pin;
    
    if (!(iss >> s_x >> s_y >> s_pin >> t_x >> t_y >> t_pin)) {
       :stderr << "ERROR: Failed to parse a path definition... exiting...";
       exit(1);
    }
  
  }

  int m_size = 2 * g_size + 1;

  for (int i; i<m_size; ++i) {
    for (int j; j<m_size; ++i) {
      if((i % 2) && (j % 2)) //(Odd, Odd) coordinate =>
        maze_array[i][j] = MazeCell(LB);
      else if((i % 2) && !(j % 2)) //(Odd, Even) coordinate =>
        maze_array[i][j] = MazeCell(VC);
      else if(!(i % 2) && (j % 2)) //(Even, Odd) coordinate =>
        maze_array[i][j] = MazeCell(HC);
      else //(Even, Even) coordinate =>
        maze_array[i][j] = MazeCell(SB);
    }
  }

  return 0;
}

