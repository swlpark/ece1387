#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <list>
#include <iterator>
#include <iostream>
#include <cassert>

struct Edge;

struct Vertex
{
  int v; 
  int x_pos; 
  int y_pos; 
  bool fixed; 
  std::list<Edge> adj_list;

  Vertex();
  void addEdge(Vertex *, float);
  void printVertex();
};

struct Edge
{
  Vertex * tgt;
  float weight;
};

#endif


