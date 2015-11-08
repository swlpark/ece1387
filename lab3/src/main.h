#ifndef _MAIN_H_
#define _MAIN_H_
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
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
#include <random>
#include "graph.h"
#include "utility.h"

extern std::vector<Vertex>              cells; 
extern std::vector<Vertex>              fixed_cells;
extern std::vector<std::vector<int>>    nets; 
extern std::vector<double>              edge_weights;
extern std::vector<std::vector<double>> Q;
extern std::vector<double>              diag_entries;
extern std::vector<Vertex>              virtual_pins;
extern int                              vp_idx;

std::vector<double> computeHPWL();
void                swapIOPair(Vertex*, unsigned int);
void                displayHPWL(std::vector<double> const &);
void                buildQ(int);
void                assignCellPos(std::vector<double> const &, int);
double              recursive_spread(std::vector<std::tuple<int,double,double>> &, double, double, std::pair<double, double>, bool, bool);
double              spread_to_zero_bin(std::vector<std::tuple<int,double,double>> &, double, double, std::pair<double, double>, bool);
double              computeOverlap(int*);
int                 countEmptyBin(int*, double, double, double);
void                assertQSymmetry(bool);

#endif
