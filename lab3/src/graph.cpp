#include "graph.h"

std::vector<std::vector<int>> Graph::nets;

Graph::Graph() : v_id(0), assigned(false), adj_nets()
{
}

void Graph::addEdge(int a_id)
{
   assert (a_id > 0);

   //return a duplicate net
   if (std::find(adj_nets.begin(), adj_nets.end(), a_id) != adj_nets.end()) {
      return;
   }
   adj_nets.push_back(a_id);
}

void Graph::printVertex()
{
  std::cout << "Adjacent nets of vertex : v=" << v_id << ", size=" << adj_nets.size();
  std::cout << "\n";
  for(auto it = adj_nets.begin(); it != adj_nets.end(); it++) 
  {
     std::cout << "Net " << (*it) << ": ";
     int net_idx = (*it) - 1;
     
     for(unsigned int i=0; i < nets.at(net_idx).size();++i)
     {
        std::cout << " -> " << nets[net_idx][i];
     }
     std::cout << "\n";
  }
  std::cout << "\n";
}
