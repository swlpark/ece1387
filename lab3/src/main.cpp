#include "main.h"

Tree* branch_and_bound(Tree*);
Tree  root;

int main(int argc, char *argv[]) {
   using namespace std;

   string   f_name;
   char arg;
   bool opt_graphics = false;
   cout << "Starting A3 application... \n" ;
   while ((arg = getopt (argc, argv, "i:g")) != -1) { switch (arg) {
         case 'i': 
            f_name = string(optarg);
            break;
         case 'g': 
            opt_graphics = true;
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
   //even number of vertices
   assert((Graph::vertices.size() % 2) == 0);

   //set-up root node, and decide vertices order
   Tree::u_set_size = Graph::vertices.size() >> 1;
   root.partition.resize(Graph::vertices.size());
   root.edge_table.resize(Graph::nets.size());
   Tree::set_partition_order(Graph::vertices);

   //optimization: assign root node to a partition
   root.partition[0] = Partition::L_ASSIGNED;
   root.L_size = 1;
   int v_idx = Tree::p2v_mapping.at(0) - 1;
   std::vector<int> & adj_nets = Graph::vertices[v_idx].adj_nets;
   for(auto it = adj_nets.begin(); it != adj_nets.end(); ++it )
   {
     int net_idx = (*it) - 1;
     if (root.edge_table[net_idx].cut_state == Partition::FREE)
     {
        root.edge_table[net_idx].cut_state = Partition::L_ASSIGNED;
     }
     else 
     {
        assert(false);
     }
   }
   root.node_idx += 1;

   std::vector<int> v_dist_sorted =Graph::vertices[(Tree::p2v_mapping[0]-1)].BFS();

   //#ifdef _DEBUG_
   for(auto i = Graph::vertices.begin(); i != Graph::vertices.end(); ++i)
   {
      (*i).printVertex();
   }
   //#endif
  
   //set an initial solution
   std::vector<Partition> init_sol(Graph::vertices.size(), Partition::R_ASSIGNED);
   for(int i=0; i < Tree::u_set_size; ++i)
   {
     init_sol[i] = Partition::L_ASSIGNED; 
   }
   Tree::u_cut_size = Tree::calc_solution_cut(init_sol, false);

   chrono::time_point<chrono::system_clock> start, end;
   start = chrono::system_clock::now();

   Tree::thread_count = 0;

   Tree* opt_sol = branch_and_bound(&root);
   end = chrono::system_clock::now();
   chrono::duration<double> delta = end - start;
   cout << "Duration of branch-and-bound partitioning: " << delta.count() << "s\n";

   assert(Tree::u_cut_size == opt_sol->cut_size);
   assert(Tree::thread_count == 4);
   Tree::calc_solution_cut(opt_sol->partition, true);
   
}

//recursive B&B
Tree* branch_and_bound(Tree * a_node)
{
   Tree *retval = nullptr;
   Tree *l_node;
   Tree *r_node;
   if (a_node->isLeaf()) {
     retval = a_node;
     if (a_node->cut_size >= Tree::u_cut_size) {
        retval = nullptr;
     } else
        Tree::u_cut_size = retval->cut_size;
   } else if (a_node->R_size == Tree::u_set_size) {
     retval = a_node->fillLeft();
     if (retval->cut_size >= Tree::u_cut_size) {
       retval = nullptr; 
     } else 
        Tree::u_cut_size = retval->cut_size;
   } else if (a_node->L_size == Tree::u_set_size) {
     retval = a_node->fillRight();
     if (retval->cut_size >= Tree::u_cut_size) {
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
         delete l_node; 
         delete r_node; 
         a_node->left_node = nullptr;
         a_node->right_node = nullptr;
         return nullptr;
       }
       std::future<Tree*> r_f, l_f;
       bool fork_r = false;
       bool fork_l = false;
       if (Tree::thread_count < 4) {
         Tree::thread_count++;
         r_f = std::async(&branch_and_bound, r_node);
         fork_r = true;
       }
       if (Tree::thread_count < 4) {
         Tree::thread_count++;
         l_f = std::async(&branch_and_bound, l_node);
         fork_l = true;
       }
       if (fork_r) r_recurse = r_f.get();
       else        r_recurse = branch_and_bound(r_node);
       if (fork_l) l_recurse = l_f.get();
       else        l_recurse = branch_and_bound(l_node);
     } else {
       if (l_node->getLowerBound() >= Tree::u_cut_size) {
         delete l_node; 
         delete r_node; 
         a_node->left_node = nullptr;
         a_node->right_node = nullptr;
         return nullptr;
       }
       std::future<Tree*> r_f, l_f;
       bool fork_r = false;
       bool fork_l = false;

       if (Tree::thread_count < 4) {
         Tree::thread_count++;
         l_f = std::async(&branch_and_bound, l_node);
         fork_l = true;
       }
       if (Tree::thread_count < 4) {
         Tree::thread_count++;
         r_f = std::async(&branch_and_bound, r_node);
         fork_r = true;
       }
       if (fork_l) l_recurse = l_f.get();
       else        l_recurse = branch_and_bound(l_node);
       if (fork_r) r_recurse = r_f.get();
       else        r_recurse = branch_and_bound(r_node);
     }
     if (l_recurse != nullptr && r_recurse != nullptr) {
        if (r_recurse->cut_size < l_recurse-> cut_size) {
            retval = r_recurse;
            delete l_recurse;
            l_node->left_node = nullptr;
            l_node->right_node = nullptr;
            delete l_node;
            a_node->left_node = nullptr;
        } else {
            retval = l_recurse;
            delete r_recurse;
            r_node->left_node = nullptr;
            r_node->right_node = nullptr;
            delete r_node;
            a_node->right_node = nullptr;
        }
     } else if (l_recurse == nullptr) {
       retval = r_recurse;
     } else if (r_recurse == nullptr) {
       retval = l_recurse;
     }
   }

   return retval;
}


