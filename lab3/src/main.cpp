#include "main.h"

//a vector of graph vertices
std::vector<Graph> vertices; 
Tree               root;

int main(int argc, char *argv[]) {
   using namespace std;

   string   f_name;
   char arg;
   cout << "Starting A3 application... \n" ;
   while ((arg = getopt (argc, argv, "i:")) != -1) {
      switch (arg) {
         case 'i': 
            f_name = string(optarg);
            break;
      }
   }

   //generate vertices & nets from the input file
   parse_test_file(f_name);

   //add nets to each vertex
   for(unsigned int i=0; i < Graph::nets.size(); ++i)
   {
      for(unsigned int j=0; j<Graph::nets[i].size(); ++j)
      {
        int v_idx = Graph::nets[i][j] - 1;
        int net_id = i+1;
        vertices[v_idx].addEdge(net_id);
      }
   }
#ifdef _DEBUG_
   for(auto i = vertices.begin(); i != vertices.end(); ++i)
   {
      (*i).printVertex();
   }
#endif
   //even number of vertices
   assert((vertices.size() % 2) == 0);

   //set-up root node, and decide vertices order
   Tree::u_set_size = vertices.size() >> 1;
   root.partition.resize(vertices.size());
   root.edge_table.resize(Graph::nets.size());
   Tree::set_partition_order(vertices);

   //choose an arbitary solution
   std::vector<Partition> init_sol(vertices.size(), Partition::R_ASSIGNED);
   for(int i=0; i < Tree::u_set_size; ++i)
   {
     init_sol[i] = Partition::L_ASSIGNED; 
   }
   Tree::u_cut_size = Tree::calc_solution_cut(init_sol);

   branch_and_bound(&root);
}

Tree* branch_and_bound(Tree * a_node)
{
   Tree *l_node = a_node->branchLeft();
   Tree *r_node = a_node->branchRight();
   Tree* r_recurse;
   Tree* l_recurse;

   if (r_node->cut_size < l_node->cut_size) {
     //prune if current cut_size (i.e. LB)
     if (r_node->getLowerBound() >= Tree::u_cut_size) {
       return nullptr;
     } else if (r_node->isLeaf()) {
       if (r_node->cut_size < Tree::u_cut_size) {
          r_node->printNode();
          Tree::u_cut_size = r_node->cut_size;
       }
       return r_node;
     }
     r_recurse = branch_and_bound(r_node);
     l_recurse = branch_and_bound(1_node);
   } else {
     branch_and_bound(l_node);
     branch_and_bound(r_node);
   }

}


