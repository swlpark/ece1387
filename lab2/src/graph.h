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
  int v_id; 
  int x_pos; 
  int y_pos; 
  bool fixed; 
  std::list<Edge> adj_list;

  Vertex();
  void addEdge(int, Vertex *, float);
  void printVertex();
};

struct Edge
{
  int e_id;
  Vertex * tgt;
  double weight;

  bool operator==(const Edge &rhs);
};

#endif


