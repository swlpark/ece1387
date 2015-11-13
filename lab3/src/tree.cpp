#include "tree.h"
int              Tree::u_cut_size;
int              Tree::u_set_size;
int              Tree::num_expansion;
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

//lookahead
int Tree::lookahead_LB()
{
  int retval = 0;
  std::list<int> l_edges;
  std::list<int> r_edges;

  //mark assigned
  for(unsigned int i = 0; i < edge_table.size(); ++i)
  {
    if(edge_table[i].cut_state == Partition::L_ASSIGNED) {
      l_edges.push_back(i+1);
    } else if(edge_table[i].cut_state == Partition::R_ASSIGNED) {
      r_edges.push_back(i+1);
    }
  }

  std::vector<std::vector<int>> hit_table;
  int row_size = l_edges.size() + r_edges.size();

  //iterate over unassigned vertices
  for(unsigned int i = node_idx; i < p2v_mapping.size(); ++i)
  {
    if(l_edges.size() == 0 || r_edges.size() == 0)
       break;
    int l_hit = 0;
    int r_hit = 0;
    int v_idx = p2v_mapping.at(i) - 1;
    std::vector<int> & adj_nets = Graph::vertices[v_idx].adj_nets;
    for(auto it = adj_nets.begin(); it != adj_nets.end(); ++it )
    {
      std::vector<int>
      int net_idx = (*it) - 1;
      if(edge_table[net_idx].cut_state == Partition::L_ASSIGNED) {
        auto l_it = std::find(l_edges.begin(), l_edges.end(), (*it));
        if (l_it != l_edges.end()) {
          ++l_hit;
          l_edges.erase(l_it);
        }
      } else if(edge_table[net_idx].cut_state == Partition::R_ASSIGNED) {
        auto r_it = std::find(r_edges.begin(), r_edges.end(), (*it));
        if (r_it != r_edges.end()) {
          ++r_hit;
          r_edges.erase(r_it);
        }
      } else {
        continue;
      }
    }
    if (l_hit > 0 && r_hit > 0) 
      retval += (l_hit > r_hit) ? r_hit : l_hit;
  }
  return retval;
}

int Tree::getLowerBound()
{
  int retval = cut_size;
  if (node_idx > 2)
    retval += lookahead_LB();
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
  num_expansion++;
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
  num_expansion++;
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

  //balance constraint
  assert(l_node->L_size == u_set_size);
  assert(l_node->R_size == u_set_size);
  assert (l_node->node_idx == (int)partition.size());
  this->left_node = l_node;
  num_expansion++;
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

  //balance constraint
  assert(r_node->L_size == u_set_size);
  assert(r_node->R_size == u_set_size);
  assert (r_node->node_idx == (int)partition.size());
  this->right_node = r_node;
  num_expansion++;
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

int Tree::calc_solution_cut(std::vector<Partition> solution, bool print)
{
   using namespace std;
   int retval = 0;
   vector<Partition> e_table;
   e_table.resize(Graph::nets.size(), Partition::FREE);

   if (print) cout << "Solution Partition:\n"; 
   for(unsigned int i =0; i<solution.size(); ++i)
   {
     if (print) cout << "v-" << p2v_mapping.at(i);
     switch (solution[i])
     {
       case Partition::FREE: 
          if (print) cout << ":F";
          break;
       case Partition::L_ASSIGNED:
          if (print) cout << ":L";
          break;
       case Partition::R_ASSIGNED:
          if (print) cout << ":R";
          break;
       case Partition::CUT:
          assert(false);
          break;
     }
     if (print) cout << "  ";
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
   if (print) cout << "\nSolution edges :\n"; 
   for(unsigned int i =0; i<e_table.size(); ++i)
   {
     if (print) cout << "Net " << i+1;
     switch (e_table[i])
     {
       case Partition::FREE: 
          if (print) cout << ": FREE\n";
          break;
       case Partition::L_ASSIGNED:
          if (print) cout << ": L_ONLY\n";
          break;
       case Partition::R_ASSIGNED:
          if (print) cout << ": R_ONLY\n";
          break;
       case Partition::CUT:
          if (print) cout << ": CUT\n";
          break;
     }
   }
   if (print) cout << "Solution cut-size: " << retval << "\n"; 
   if (print) cout << "Number of expansions: " << num_expansion << "\n"; 

   assert((unsigned int)retval <= Graph::nets.size());
   return retval;
}
