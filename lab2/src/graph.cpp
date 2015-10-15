#include "graph.h"

Vertex::Vertex() : v(0), x_pos(0), y_pos(0), fixed(false), adj_list()
{
}

void Vertex::addEdge(Vertex * a_tgt, float a_weight)
{
   assert (a_tgt != nullptr);
   assert (a_weight > 0);

   Edge e;
   e.tgt = a_tgt; 
   e.weight = a_weight; 
   adj_list.push_back(e);
}

void Vertex::printVertex()
{
  std::cout << "Adjacent cells of Vertex - v=" << v << ", size=" << adj_list.size() << "\n";
  for(auto i = adj_list.begin(); i != adj_list.end(); i++) 
  {
     std::cout << " -> " << (*i).tgt->v;
  }
  std::cout << "\n";
}
