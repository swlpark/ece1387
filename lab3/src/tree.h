#ifndef _TREE_H_
#define _TREE_H_

#include <vector>
#include <list>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <cassert>
#include "graph.h"
#include "main.h"

enum Partition {FREE=0, L_ASSIGNED, R_ASSIGNED, CUT};

struct Edge
{
  Partition        cut_state;
  //std::vector<int> p_idx_list;
};

//branch-and-bound partial solution tree node
struct Tree
{
  //idx to the node's partition slot (i.e. a newly assigned vertex)
  int                    node_idx;
  std::vector<Partition> partition;
  int                    cut_size;
  int                    L_size;
  int                    R_size;

  //each entry is assoicated with a net;
  std::vector<Edge>      edge_table;

  //pointers to B&B sub-problems
  Tree*                  left_node;
  Tree*                  right_node;

  Tree();
  ~Tree();
  int  getLowerBound();
  void printNode();
  bool isLeaf();

  Tree* branchLeft();
  Tree* branchRight();

  //expand one side to the Leaf node to meet balance constraint
  Tree* fillLeft();
  Tree* fillRight();

  //current best solution's cut size
  static int u_cut_size;
  //balance constraint
  static int u_set_size;

  //maps which vertex is mapped on each partition slot
  //(i.e. sorted vertices, except we only keep v_id)
  static std::vector<int> p2v_mapping;

  //assign p2v_mapping vector
  static void set_partition_order(std::vector<Graph>);

  //calculate cut-size of a complete solution
  static int calc_solution_cut(std::vector<Partition>);
};
#endif
