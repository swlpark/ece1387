#include "graph.h"

std::vector<int> Vertex::v_map_table;

Vertex::Vertex() : v_id(0), x_pos(0), y_pos(0), fixed(false), v_pin(false), adj_list()
{
}

void Vertex::addEdge(int a_id, Vertex * a_tgt, float a_weight)
{
   assert (a_tgt != nullptr);
   assert (a_weight > 0);

   Edge e;
   e.e_id = a_id; 
   e.tgt = a_tgt; 
   e.weight = a_weight; 

   //ignores an edge to itself unless it is a virtual pin
   if ((a_tgt->v_id == this->v_id) && !v_pin) {
#ifdef _DEBUG_
      std::cout << "DEBUG: Vertex v=" << v_id << "ignores edge to itself \n";
#endif
      return;
   }
 
   //finds a duplicate edge in the adj list
   //NOTE: may have duplicate edges between vertices as long as edges come from different nets
   if (std::find(adj_list.begin(), adj_list.end(), e) != adj_list.end()) {
      return;
   }
   adj_list.push_back(e);
}

void Vertex::printVertex()
{
  std::cout << "Adjacent cells of Vertex : v=" << v_id << ", size=" << adj_list.size();

  if (fixed)
    std::cout << "; (FIXED_CELL)";

  std::cout << "\n";
  for(auto i = adj_list.begin(); i != adj_list.end(); i++) 
  {
     std::cout << " -> " << (*i).tgt->v_id << "(w=" << (*i).weight << ")";
  }
  std::cout << "\n";
}

bool Edge::operator==(const Edge &rhs)
{
   if (tgt == rhs.tgt && e_id == rhs.e_id)
      return true;
   else 
      return false;
}
