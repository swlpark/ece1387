#ifndef _MAIN_H_
#define _MAIN_H_
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <unistd.h>
#include <cstdlib>
#include <cassert>
#include <ctime>
#include <cmath>
#include "graph.h"
#include "utility.h"

extern std::vector<Vertex>              cells; 
extern std::vector<Vertex>              fixed_cells;
extern std::vector<std::vector<int>>    nets; 
extern std::vector<double>              edge_weights;
extern std::vector<std::vector<double>> Q;
extern std::vector<double>              diag_entries;

#endif
