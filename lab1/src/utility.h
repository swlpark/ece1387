#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <vector>
#include <string>
#include <thread>
#include <iostream>
#include <sstream>
#include "graphics.h"
#include "grid_net.h"
#include "grid_cell.h"
#include "fpga_router.h"
#include "utility.h"


void build_FPGA_grid (std::vector<std::vector<GridCell>> &grid, int grid_dim);
int  matchAdjacentPin (int src_o_pin, GridCell * source , GridCell * target);

//Graphics-related
void drawscreen (void);
void begin_graphics (void);
#endif

