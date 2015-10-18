#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <list>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <cassert>

struct Edge;

struct Vertex
{
  int    v_id; 
  double x_pos; 
  double y_pos; 
  bool fixed; 
  std::list<Edge> adj_list;

  Vertex();
  void addEdge(int, Vertex *, float);
  void printVertex();

  //maps v to a slot in Q matrix 
  static std::vector<int> v_map_table;
};

struct Edge
{
  int e_id;
  Vertex * tgt;
  double weight;

  bool operator==(const Edge &rhs);
};

#endif


