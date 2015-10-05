#include "grid_net.h"

int GridNet::s_branch_num = 4;

GridNet::GridNet() : m_cr_graph() {
   m_net_id = 0;

   m_src_x = 0;
   m_src_y = 0;
   m_src_p = 0;

   m_tgt_x = 0;
   m_tgt_y = 0;
   m_tgt_p = 0;

   m_dr_graph  = nullptr;
   m_line_dist = 0;
}

GridNet::GridNet(int _id, int _s_x, int _s_y, int _s_p, int _t_x, int _t_y, int _t_p) : m_cr_graph() {
   m_net_id = _id;

   m_src_x = _s_x;
   m_src_y = _s_y;
   m_src_p = _s_p;

   m_tgt_x = _t_x;
   m_tgt_y = _t_y;
   m_tgt_p = _t_p;

   m_dr_graph  = nullptr;
   m_line_dist = sqrt((pow(abs(m_tgt_x - m_src_x), 2) + pow(abs(m_tgt_y - m_src_y), 2)));
}

//delete call is recursive
GridNet::~GridNet() {
  //traverse tree and delete
  delete m_dr_graph;
}

int GridNet::getLineDistance(){
   return m_line_dist;
}

Coordinate GridNet::getSrcCoordinate(){
   Coordinate retval(m_src_x, m_src_y, m_src_p);
   return retval;
}

Coordinate GridNet::getTgtCoordinate(){
   Coordinate retval(m_tgt_x, m_tgt_y, m_tgt_p);
   return retval;
}

int GridNet::generateDrTree() {
   if (m_cr_graph.size() < 3) {
      std::cerr << "GridNet Error: must have +3 nodes on a coarse graph path\n";
      return EXIT_FAILURE;
   }

   std::list<PathTree*> leaf_queue;
   for (auto it = m_cr_graph.begin(); it != m_cr_graph.end(); ++it) {
      PathTree* parent;
      PathTree* exp_node;
      GridCell * lh_cell =  *(std::next(it, 1));

      if ((*it)-> m_type == CellType::V_CHANNEL || (*it)->m_type == CellType::H_CHANNEL ) {
         //branch node at first channel on path
         if (leaf_queue.size() == 1 && leaf_queue.front()->node_ptr->m_type == CellType::LOGIC_BLOCK) {
            parent = leaf_queue.front();
            leaf_queue.pop_front();

            std::vector<int> exp_pins;
            if ((*it)->getTrackBundle(s_branch_num, lh_cell, exp_pins) > 0) {
               PathTree* br_node = new PathTree();
               br_node->node_ptr = (*it);
               br_node->grp_number = 0;
               br_node->o_pins = std::move(exp_pins);
               br_node->children.reserve(br_node->o_pins.size());
               parent->children.push_back(br_node);
               leaf_queue.push_back(br_node);
            } else { //ROUTING FAIL
               std::cout << "GridNet Info: not enough routing resource(s) on the first channel\n";
               return EXIT_FAILURE;
            }
         } else if (leaf_queue.size() >= 1) { //expanding each path by one hop
            while (!leaf_queue.empty()) {
               parent = leaf_queue.front();
               leaf_queue.pop_front();
               PathTree* exp_node = new PathTree();
               exp_node->node_ptr = (*it);
               exp_node->grp_number = parent->grp_number;
               exp_node->o_pins.reserve(1);
               exp_node->children.reserve(1);
               parent->children.push_back(exp_node);

               //mapping parent's o_pin to target's input pin
               int tgt_i_pin = matchAdjacentPin(parent->o_pins[0], parent->node_ptr, (*it));
               if (tgt_i_pin >= 0) {
                  int tgt_o_pin;
                  tgt_o_pin =(*it)->getOutputPin(tgt_i_pin, lh_cell);
                  if (tgt_o_pin >= 0) {
                     exp_node->o_pins.push_back(tgt_o_pin);
                  } else {
                     std::cout << "GridNet Info: not enough routing resource(s) on a middle channel\n";
                     return EXIT_FAILURE;
                  }
               } else {
                  std::cerr << "GridNet Error: could not find a matching pin...\n";
                  return EXIT_FAILURE;
               }
            }
         } else {
            std::cerr << "GridNet Error: Channel is encountered when queue is empty\n";
            return EXIT_FAILURE;
         }
      } else if ((*it)-> m_type == CellType::SWITCH_BOX) {
         //branch childen on first switch box
         if (leaf_queue.size() == 1 && leaf_queue.front()->o_pins.size() > 1) {
            parent = leaf_queue.front();
            leaf_queue.pop_front();
            int grp_num = 0;

            for(auto pin_it = parent->o_pins.begin(); pin_it != parent->o_pins.end(); ++pin_it) {
               exp_node = new PathTree();
               exp_node->node_ptr = (*it);
               exp_node->grp_number = ++grp_num;
               exp_node->o_pins.reserve(1);
               exp_node->children.reserve(1);
               parent->children.push_back(exp_node);

               //mapping parent's o_pin to target's input pin
               int tgt_i_pin = matchAdjacentPin(*pin_it, parent->node_ptr, (*it));
               if (tgt_i_pin >= 0) {
                  int tgt_o_pin;
                  tgt_o_pin =(*it)->getOutputPin(tgt_i_pin, lh_cell);
                  if (tgt_o_pin >= 0) {
                     exp_node->o_pins.push_back(tgt_o_pin);
                  } else {
                     std::cout << "GridNet Info: not enough routing resource(s) on a middle channel\n";
                     return EXIT_FAILURE;
                  }
               } else {
                  std::cerr << "GridNet Error: could not find a matching pin...\n";
                  return EXIT_FAILURE;
               }

            }
         } else if (leaf_queue.size() >= 1) { //expanding each path by one hop
            while (!leaf_queue.empty()) {
               parent = leaf_queue.front();
               leaf_queue.pop_front();
               PathTree* exp_node = new PathTree();
               exp_node->node_ptr = (*it);
               exp_node->grp_number = parent->grp_number;
               exp_node->o_pins.reserve(1);
               exp_node->children.reserve(1);
               parent->children.push_back(exp_node);

               //mapping parent's o_pin to target's input pin
               int tgt_i_pin = matchAdjacentPin(parent->o_pins[0], parent->node_ptr, (*it));
               if (tgt_i_pin >= 0) {
                  int tgt_o_pin;
                  tgt_o_pin =(*it)->getOutputPin(tgt_i_pin, lh_cell);
                  if (tgt_o_pin >= 0) {
                     exp_node->o_pins.push_back(tgt_o_pin);
                  } else {
                     std::cout << "GridNet Info: not enough routing resource(s) on a middle channel\n";
                     return EXIT_FAILURE;
                  }
               } else {
                  std::cerr << "GridNet Error: could not find a matching pin...\n";
                  return EXIT_FAILURE;
               }
            }
         } else {
            std::cerr << "GridNet Error: Channel is encountered when queue is empty\n";
            return EXIT_FAILURE;
         }
      } else if ((*it)-> m_type == CellType::LOGIC_BLOCK) {
         //first element
         if (it == m_cr_graph.begin() && m_dr_graph == nullptr) {
            m_dr_graph = new PathTree();
            m_dr_graph->node_ptr = (*m_cr_graph.begin());
            m_dr_graph->grp_number = 0;
            m_dr_graph->path_cost = 0;
            m_dr_graph->o_pins.reserve(1);
            m_dr_graph->o_pins[0] = m_src_p;
            m_dr_graph->children.reserve(1);
            leaf_queue.push_back(m_dr_graph);
         } else if ((it != m_cr_graph.end()) && (it == --m_cr_graph.end()))  { //LAST ELEMENT
            while (!leaf_queue.empty()) {
               parent = leaf_queue.front();
               leaf_queue.pop_front();
               PathTree* exp_node = new PathTree();
               exp_node->node_ptr = (*it);
               exp_node->grp_number = parent->grp_number;
               parent->children.push_back(exp_node);
            }
            break;
         } else {
            std::cerr << "GridNet Error: LB in the middle of cr_graph\n";
            return EXIT_FAILURE;
         }
      }
   }
   return EXIT_SUCCESS;
}

