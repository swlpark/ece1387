#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <list>
#include <vector>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <cassert>

struct Graph
{
  int              v_id; 
  bool             assigned; 
  std::vector<int> adj_nets;

  Graph();
  void addEdge(int);
  void printVertex();

  static std::vector<std::vector<int>> nets;
};

#endif


