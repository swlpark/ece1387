#ifndef _GRID_NET_H_
#define _GRID_NET_H_
#define EXPAND_FAIL -1

#include <iostream>
#include <cstdlib>
#include <iterator>
#include <vector>
#include <cmath>
#include <ctime>
#include <list>
#include "utility.h"
#include "grid_cell.h"
 
struct Coordinate {
   int x;
   int y;
   int p;
   Coordinate() : x(0), y(0), p(0) {}
   Coordinate(int _x, int _y, int _p) : x(_x), y(_y), p(_p) {}

   bool operator==(const Coordinate &rhs) {
      if (x == rhs.x && y == rhs.y && p == rhs.p)
         return true;
      else 
         return false;
   }
};

class GridNet {
   int m_line_dist;

   //Pin vector (i.e. edges on m_graph)
   //------------------------------
   std::vector<int>       o_pins;

   int        connectPins();

   public:
   int m_net_id;
   int m_routed;

   //Coarse-Routed Graph (i.e. List of connected cells)
   std::list<GridCell*> m_graph;

   //Source to Target
   int m_src_x, m_src_y, m_src_p;
   int m_tgt_x, m_tgt_y, m_tgt_p;

   static int s_branch_num; //Net branch expand size

   GridNet();
   GridNet(int, int, int, int, int, int, int);
   ~GridNet();

   int        getLineDistance();
   Coordinate getSrcCoordinate();
   Coordinate getTgtCoordinate();
   void       insertNode(GridCell * node);
   bool       routeGraph(int, int);
   void       printGraph();
};
#endif

