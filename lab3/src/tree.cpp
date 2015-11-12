#include "tree.h"

int              Tree::u_cut_size;
int              Tree::u_set_size;
std::vector<int> Tree::p2v_mapping;

//higher vertices
struct sort_vertices {
   bool operator()(const Graph &lhs, const Graph &rhs) {
     return (lhs.adj_nets.size() > rhs.adj_nets.size());
   }
};

Tree::Tree() : node_idx(0), partition(), cut_size(0), L_size(0), R_size(0), edge_table()
{
  left_node = nullptr;
  right_node = nullptr;
}

Tree::~Tree()
{
  if (left_node != nullptr)
    delete left_node;
  if (right_node != nullptr)
    delete right_node;
}

int Tree::getLowerBound()
{
  int retval = cut_size;
  return retval;
}

Tree* Tree::branchLeft()
{
  assert (node_idx < (int)partition.size());
  //Leaf node
  if (node_idx == ((int)partition.size() - 1))
     return this;

  Tree* l_node = new Tree();

  l_node->partition = this->partition;
  l_node->node_idx = this->node_idx;
  l_node->partition[l_node->node_idx] = Partition::L_ASSIGNED;
  l_node->cut_size = this->cut_size;
  l_node->edge_table = this->edge_table;

  l_node->L_size = this->L_size + 1;
  l_node->R_size = this->R_size;

  //balance constraint
  assert(l_node->L_size <= u_set_size);
  assert(l_node->R_size <= u_set_size);

  //update cut_size & edge_table accordingly
  int v_idx = p2v_mapping.at(l_node->node_idx) - 1;
  std::vector<int> & adj_nets = Graph::vertices[v_idx].adj_nets;
  for(auto it = adj_nets.begin(); it != adj_nets.end(); ++it )
  {
    int net_idx = (*it) - 1;
    if (l_node->edge_table[net_idx].cut_state == Partition::FREE)
    {
       l_node->edge_table[net_idx].cut_state = Partition::L_ASSIGNED;
    }
    else if (l_node->edge_table[net_idx].cut_state == Partition::R_ASSIGNED)
    {
       l_node->edge_table[net_idx].cut_state = Partition::CUT;
       l_node->cut_size += 1;
    }
  }

  l_node->node_idx += 1;
  this->left_node = l_node;
  return l_node;
}

Tree* Tree::branchRight()
{
  assert (node_idx < (int)partition.size());
  //Leaf node
  if (node_idx == ((int)partition.size() - 1))
     return this;

  Tree* r_node = new Tree();

  r_node->partition = this->partition;
  r_node->node_idx = this->node_idx;
  r_node->partition[r_node->node_idx] = Partition::R_ASSIGNED;
  r_node->cut_size = this->cut_size;
  r_node->edge_table = this->edge_table;

  r_node->L_size = this->L_size;
  r_node->R_size = this->R_size + 1;

  //balance constraint
  assert(r_node->L_size <= u_set_size);
  assert(r_node->R_size <= u_set_size);

  //update cut_size & edge_table accordingly
  int v_idx = p2v_mapping.at(r_node->node_idx) - 1;
  std::vector<int> & adj_nets = Graph::vertices[v_idx].adj_nets;
  for(auto it = adj_nets.begin(); it != adj_nets.end(); ++it )
  {
    int net_idx = (*it) - 1;
    if (r_node->edge_table[net_idx].cut_state == Partition::FREE)
    {
       r_node->edge_table[net_idx].cut_state = Partition::R_ASSIGNED;
    }
    else if (r_node->edge_table[net_idx].cut_state == Partition::L_ASSIGNED)
    {
       r_node->edge_table[net_idx].cut_state = Partition::CUT;
       r_node->cut_size += 1;
    }
  }

  r_node->node_idx += 1;
  this->right_node = r_node;
  return r_node;
}

Tree* Tree::fillLeft()
{
  int fill_size = u_set_size - L_size; 
  Tree* l_node = new Tree();
  l_node->node_idx = this->node_idx;
  l_node->partition = this->partition;
  l_node->cut_size = this->cut_size;
  l_node->edge_table = this->edge_table;
  l_node->L_size = this->L_size;
  l_node->R_size = this->R_size;

  //Assign free vertices to Left Partition; update edge table
  for(int i = 0; i < fill_size; ++i) {
   l_node->partition[l_node->node_idx] = L_ASSIGNED;
   l_node->L_size += 1;

   int v_idx = p2v_mapping.at(l_node->node_idx) - 1;
   std::vector<int> & adj_nets = Graph::vertices[v_idx].adj_nets;
   for(auto it = adj_nets.begin(); it != adj_nets.end(); ++it )
   {
     int net_idx = (*it) - 1;
     switch (l_node->edge_table[net_idx].cut_state)
     {
       case Partition::FREE: 
          l_node->edge_table[net_idx].cut_state = Partition::L_ASSIGNED;
          break;
       case Partition::L_ASSIGNED:
          //no-op
          break;
       case Partition::R_ASSIGNED:
          l_node->edge_table[net_idx].cut_state = Partition::CUT;
          l_node->cut_size += 1;
          break;
       case Partition::CUT:
          break;
     }
   }
   l_node->node_idx += 1;
  }
  //l_node->L_size = this->L_size + fill_size;
  //l_node->node_idx += fill_size;

  //balance constraint
  assert(l_node->L_size == u_set_size);
  assert(l_node->R_size == u_set_size);
  assert (l_node->node_idx == (int)partition.size());
  return l_node;
}

Tree* Tree::fillRight()
{
  int fill_size = u_set_size - R_size; 
  Tree* r_node = new Tree();
  r_node->node_idx = this->node_idx;
  r_node->partition = this->partition;
  r_node->cut_size = this->cut_size;
  r_node->edge_table = this->edge_table;
  r_node->L_size = this->L_size;
  r_node->R_size = this->R_size;

  //Assign free vertices to Left Partition; update edge table
  for(int i = 0; i < fill_size; ++i) {
   r_node->partition[r_node->node_idx] = R_ASSIGNED;
   r_node->R_size += 1;

   int v_idx = p2v_mapping.at(r_node->node_idx) - 1;
   std::vector<int> & adj_nets = Graph::vertices[v_idx].adj_nets;
   for(auto it = adj_nets.begin(); it != adj_nets.end(); ++it )
   {
     int net_idx = (*it) - 1;
     switch (r_node->edge_table[net_idx].cut_state)
     {
       case Partition::FREE: 
          r_node->edge_table[net_idx].cut_state = Partition::L_ASSIGNED;
          break;
       case Partition::L_ASSIGNED:
          r_node->edge_table[net_idx].cut_state = Partition::CUT;
          r_node->cut_size += 1;
          break;
       case Partition::R_ASSIGNED:
          //no-op
          break;
       case Partition::CUT:
          break;
     }
   }
   r_node->node_idx += 1;
  }
  //r_node->R_size = this->R_size + fill_size;
  //r_node->node_idx += fill_size;

  //balance constraint
  assert(r_node->L_size == u_set_size);
  assert(r_node->R_size == u_set_size);
  assert (r_node->node_idx == (int)partition.size());
  return r_node;
}



void Tree::printNode()
{
  using namespace std;
  cout << "node_idx=" << node_idx << "; cut_size=" << cut_size << "; L_size=" << L_size \
       << "; R_size=" << R_size << "\n"; 
  cout << "Node Partition:\n"; 
  for(unsigned int i =0; i<partition.size(); ++i)
  {
    cout << "v-" << p2v_mapping.at(i);
    switch (partition[i])
    {
      case Partition::FREE: 
         cout << ":F";
         break;
      case Partition::L_ASSIGNED:
         cout << ":L";
         break;
      case Partition::R_ASSIGNED:
         cout << ":R";
         break;
      case Partition::CUT:
         assert(false);
         break;
    }
    cout << "  ";
  }
  cout << "\nEdge Table:\n"; 
  for(unsigned int i =0; i<edge_table.size(); ++i)
  {
    cout << "Net " << i+1;
    switch (edge_table[i].cut_state)
    {
      case Partition::FREE: 
         cout << ": FREE\n";
         break;
      case Partition::L_ASSIGNED:
         cout << ": L_ONLY\n";
         break;
      case Partition::R_ASSIGNED:
         cout << ": R_ONLY\n";
         break;
      case Partition::CUT:
         cout << ": CUT\n";
         break;
    }
  }
}

bool Tree::isLeaf()
{
  bool retval = false;
  assert(node_idx < (int)Graph::vertices.size());
  if (node_idx == (int)Graph::vertices.size()) {
    retval = true; 
    assert(L_size == u_set_size);
    assert(R_size == u_set_size);
    for(unsigned int i=0; i < Graph::vertices.size(); ++i)
    {
      assert(partition[i] != Partition::FREE && partition[i] != Partition::CUT);
    }
  }
  return retval;
}

void Tree::set_partition_order(std::vector<Graph> graph)
{
  p2v_mapping.resize(graph.size());
  std::sort(graph.begin(), graph.end(), sort_vertices());
  for(unsigned int i=0; i<graph.size(); i=i+1)
  {
    p2v_mapping[i] = graph[i].v_id; 
  }
}

int Tree::calc_solution_cut(std::vector<Partition> solution)
{
   using namespace std;
   int retval = 0;
   vector<Partition> e_table;
   e_table.resize(Graph::nets.size(), Partition::FREE);

   cout << "Solution Partition:\n"; 
   for(unsigned int i =0; i<solution.size(); ++i)
   {
     cout << "v-" << p2v_mapping.at(i);
     switch (solution[i])
     {
       case Partition::FREE: 
          cout << ":F";
          break;
       case Partition::L_ASSIGNED:
          cout << ":L";
          break;
       case Partition::R_ASSIGNED:
          cout << ":R";
          break;
       case Partition::CUT:
          assert(false);
          break;
     }
     cout << "  ";
   }

   //iterate over vertices
   for(unsigned int i=0; i<solution.size(); i++)
   {
     assert(solution[i] != Partition::FREE && solution[i] != Partition::CUT);
     int v_idx = p2v_mapping.at(i) - 1;
     vector<int> & adj_nets = Graph::vertices[v_idx].adj_nets;
     for(auto it = adj_nets.begin(); it != adj_nets.end(); ++it )
     {
       int net_idx = (*it) - 1;
       switch (e_table[net_idx])
       {
         case Partition::FREE: 
            if (solution[i] == Partition::L_ASSIGNED) {
              e_table[net_idx] = Partition::L_ASSIGNED;
            } else {
              e_table[net_idx] = Partition::R_ASSIGNED;
            }
            break;
         case Partition::L_ASSIGNED:
            if (solution[i] == Partition::R_ASSIGNED) {
              e_table[net_idx] = Partition::CUT;
              ++retval;
            }
            break;
         case Partition::R_ASSIGNED:
            if (solution[i] == Partition::L_ASSIGNED) {
              e_table[net_idx] = Partition::CUT;
              ++retval;
            }
            break;
         case Partition::CUT:
            break;
       }
     }
   }
   cout << "\nSolution edges :\n"; 
   for(unsigned int i =0; i<e_table.size(); ++i)
   {
     cout << "Net " << i+1;
     switch (e_table[i])
     {
       case Partition::FREE: 
          cout << ": FREE\n";
          break;
       case Partition::L_ASSIGNED:
          cout << ": L_ONLY\n";
          break;
       case Partition::R_ASSIGNED:
          cout << ": R_ONLY\n";
          break;
       case Partition::CUT:
          cout << ": CUT\n";
          break;
     }
   }
   cout << "Solution cut-size: " << retval << "\n"; 

   assert((unsigned int)retval <= Graph::nets.size());
   return retval;
}


