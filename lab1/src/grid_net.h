#ifndef _GRID_NET_H_
#define _GRID_NET_H_
#define EXPAND_FAIL -1

#include <iostream>
#include <iterator>
#include <vector>
#include <cmath>
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
   struct PathTree {
      GridCell               *node_ptr;
      int                    path_cost;
      int                    grp_number;
      std::vector<PathTree*> children;     
      std::vector<int>       o_pins;

      PathTree () : children(), o_pins() {
         node_ptr = nullptr;
         path_cost = 0;
         grp_number = 0;
      }
      ~PathTree () {
         for(auto it= children.begin(); it != children.end(); ++it) {
             delete *it;
         }
      }

   };
   int m_line_dist;

   //Detail-Routed Graph (i.e Tree)
   //------------------------------
   PathTree * m_dr_graph;

   public:
   int m_net_id;

   //Source to Target
   int m_src_x, m_src_y, m_src_p;
   int m_tgt_x, m_tgt_y, m_tgt_p;
   int m_tgt_row, m_target_col, m_target_pin;

   //Coarse-Routed Graph (i.e. List)
   std::list<GridCell*> m_cr_graph;

   static int s_branch_num; //Net branch expand size

   GridNet();
   GridNet(int, int, int, int, int, int, int);
   ~GridNet();

   int        generateDrTree();
   int        getLineDistance();
   Coordinate getSrcCoordinate();
   Coordinate getTgtCoordinate();
   void       printCrGraph();
};

#endif

