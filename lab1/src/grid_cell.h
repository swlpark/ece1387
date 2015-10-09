#ifndef _GRID_CELL_H_
#define _GRID_CELL_H_
#define EXPAND_FAIL -1

#include <list>
#include <limits>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <string>

//Different from LB pin definition from the input definition
enum CellSide {SOUTH=0, EAST=1, NORTH=2, WEST=3};
enum class CellType {LOGIC_BLOCK, V_CHANNEL, H_CHANNEL, SWITCH_BOX};

//forward declaration
class GridNet;
class GridCell {
      struct CellPin {
         bool    routed;
         //int     net_idx;      //idx to routed net in net_list
         int     net_ref_cnt;  //incremented on every expand call made by individual CellNets
      };

      std::list  <GridNet*>  m_net_list;
      int      __calcCellCost(bool);

   public:
      //Cell Co-ordinates on Grid
      int               m_adj_cnt;

      int               m_x_pos;
      int               m_y_pos;
      CellType          m_type;

      GridCell          *m_adj_south;
      GridCell          *m_adj_east;
      GridCell          *m_adj_north;
      GridCell          *m_adj_west;

      std::vector<CellPin>   m_pin_list;

      //------------------------------
      //Dikstra back-tracking ptr
      //------------------------------

      int         m_cr_path_cost;
      GridCell   *m_cr_pred;
      bool        m_cr_reached;
      int         m_cr_track;

      static int               s_ch_width;
      static bool              s_uni_track;
      static std::vector<int>  s_track_ref;

      GridCell();
      GridCell(int, int) ;
      ~GridCell();

      int             setAdjacency  (GridCell *, GridCell *, GridCell *, GridCell *); 
      bool operator < (const GridCell&) const;

      //return "global congestion" cost of using this cell
      int                    getCellCost (int, int, int, int, const GridCell *);
      std::vector<GridCell*> getAdjCells(int);                      

      int                    addNet     (GridNet *);       //Add a net to the cell
      void                   removeNet  (GridNet *);       //remove a net assigned to this cell
      int                    burnPin    (int);             //tag the pin as occupied

      int           getTracks (int *);
      //CellNet will call getDrEdges on C and S cells; returns number of edges written to vector
      int           getTrackBundle  (int, const GridCell *, std::vector<int> &); //called when S->C
      int           getOutputPin    (int, int, const GridCell *); //"expand given an input pin, and target cell" 
      int           routeDrNet   (int, int) ;           //trigger update to attached nets
};

#endif


