#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <vector>
#include <list>
#include <string>
#include <thread>
#include <iostream>
#include <sstream>
#include <iterator>
#include <utility>
#include <unordered_set>

#include "graph.h"
#include "umfpack.h"
#include "graphics.h"
#include "main.h"

//analytical placement related
std::vector<double> solveQ(std::vector<std::vector<double>>const &, std::vector<Vertex> const &);

//graphics related
void begin_graphics (void);
void drawscreen     (void);

#endif

