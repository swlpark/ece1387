#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <vector>
#include <list>
#include <string>
#include <thread>
#include <iostream>
#include <sstream>
#include <iterator>

#include "graph.h"
#include "umfpack.h"
#include "graphics.h"

//analytical placement related
std::vector<double> solveQ(std::vector<std::vector<double>>const &, std::vector<Vertex> const &);

//graphics related
void drawscreen     (void);
void begin_graphics (void);

#endif

