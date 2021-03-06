#include "graph.h"

std::vector<Graph> Graph::vertices;
std::vector<std::vector<int>> Graph::nets;

Graph::Graph() : v_id(0), v_dist(-1), assigned(false), adj_nets()
{
}

/*
* Annotate each vertex with minimum distance from the this node,
* return a vector of nodes sorted in BFS sorted order.
*/
std::vector<int> Graph::BFS()
{
  std::vector<int> retval;

  v_dist = 0;
  std::list<int> queue;
  queue.push_back((this->v_id - 1));
  retval.push_back(v_id);
  int idx;
  while(!queue.empty()) 
  {
     idx = queue.front(); 
     queue.pop_front(); 
     for(auto it = vertices[idx].adj_nets.begin(); it != vertices[idx].adj_nets.end(); it++) 
     {
        int net_idx = (*it) - 1;
        //vertices connected by the net
        for(unsigned int i=0; i < nets.at(net_idx).size();++i)
        {
           int adj_idx = nets[net_idx][i] - 1;

           if (vertices[adj_idx].v_dist == -1) {
             vertices[adj_idx].v_dist = vertices[idx].v_dist + 1;
             queue.push_back(adj_idx);
             retval.push_back(adj_idx + 1);
           }
        }
     }
  }
  assert(retval.size() == vertices.size());
  return retval;
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
  std::cout << "Vertex : v=" << v_id <<  ", dist=" << v_dist << ", adj_nets.size=" << adj_nets.size();
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
