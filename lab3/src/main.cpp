#include "main.h"

Tree* branch_and_bound(Tree*);
Tree  root;

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
        Graph::vertices[v_idx].addEdge(net_id);
      }
   }
//#ifdef _DEBUG_
   for(auto i = Graph::vertices.begin(); i != Graph::vertices.end(); ++i)
   {
      (*i).printVertex();
   }
//#endif
   //even number of vertices
   assert((Graph::vertices.size() % 2) == 0);

   //set-up root node, and decide vertices order
   Tree::u_set_size = Graph::vertices.size() >> 1;
   root.partition.resize(Graph::vertices.size());
   root.edge_table.resize(Graph::nets.size());
   Tree::set_partition_order(Graph::vertices);

   //choose an arbitary solution
   std::vector<Partition> init_sol(Graph::vertices.size(), Partition::R_ASSIGNED);
   for(int i=0; i < Tree::u_set_size; ++i)
   {
     init_sol[i] = Partition::L_ASSIGNED; 
   }
   Tree::u_cut_size = Tree::calc_solution_cut(init_sol);

   Tree* opt_sol = branch_and_bound(&root);
   assert(Tree::u_cut_size == opt_sol->cut_size);
   Tree::calc_solution_cut(opt_sol->partition);
   
}

//recursive B&B
Tree* branch_and_bound(Tree * a_node)
{
   Tree *retval = nullptr;
   Tree *l_node;
   Tree *r_node;
   if (a_node->isLeaf()) {
     retval = a_node;
#ifdef _DEBUG_
     a_node->printNode();
#endif
     if (a_node->cut_size >= Tree::u_cut_size) {
#ifdef _DEBUG_
        std::cout << "Pruning a Leaf node; cut_size >= U(" << Tree::u_cut_size << ")\n";
#endif
        retval = nullptr;
     } else
        Tree::u_cut_size = retval->cut_size;
   } else if (a_node->R_size == Tree::u_set_size) {
     retval = a_node->fillLeft();
#ifdef _DEBUG_
     retval->printNode();
#endif
     if (retval->cut_size >= Tree::u_cut_size) {
#ifdef _DEBUG_
       std::cout << "Pruning a Leaf node; cut_size >= U(" << Tree::u_cut_size << ")\n";
#endif
       retval = nullptr; 
     } else 
        Tree::u_cut_size = retval->cut_size;
   } else if (a_node->L_size == Tree::u_set_size) {
     retval = a_node->fillRight();
#ifdef _DEBUG_
     retval->printNode();
#endif
     if (retval->cut_size >= Tree::u_cut_size) {
#ifdef _DEBUG_
       std::cout << "Pruning a Leaf node; cut_size >= U(" << Tree::u_cut_size << ")\n";
#endif
       retval = nullptr; 
     } else 
        Tree::u_cut_size = retval->cut_size;
   } else { //recursive Tree expansion
     Tree *r_recurse;
     Tree *l_recurse;
     l_node = a_node->branchLeft();
     r_node = a_node->branchRight();

     //prune if LB is equal or greater than U
     if (r_node->getLowerBound() < l_node->getLowerBound()) {
       if (r_node->getLowerBound() >= Tree::u_cut_size) {
#ifdef _DEBUG_
         std::cout << "Pruning intra-nodes; LB >= U(" << Tree::u_cut_size << ")\n";
#endif
         return nullptr;
       }
       r_recurse = branch_and_bound(r_node);
       l_recurse = branch_and_bound(l_node);
     } else {
       if (l_node->getLowerBound() >= Tree::u_cut_size) {
#ifdef _DEBUG_
         std::cout << "Pruning intra-nodes; LB >= U(" << Tree::u_cut_size << ")\n";
#endif
         return nullptr;
       }
       l_recurse = branch_and_bound(l_node);
       r_recurse = branch_and_bound(r_node);
     }
     if (l_recurse != nullptr && r_recurse != nullptr) {
        if (r_recurse->cut_size < l_recurse-> cut_size)
            retval = r_recurse;
        else 
            retval = l_recurse;
     } else if (l_recurse == nullptr) {
       retval = r_recurse;
     } else if (r_recurse == nullptr) {
       retval = l_recurse;
     }
   }

   return retval;
}


