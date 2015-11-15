#include "utility.h"

//**************************************************************************
//*file I/O: parse input file, set vertices & nets
//**************************************************************************
void parse_test_file(std::string f_name)
{
   using namespace std;
   ifstream in_file;
   in_file.open(f_name);
   if (!in_file.is_open()) {
      cerr << "Cannot open file - " << f_name << "; please check file exists\n";
      exit(EXIT_FAILURE);
   }

   string line;
   while(getline(in_file,line))
   {
      istringstream iss(line);
      int value;
      if (!(iss >> value)) {
         cerr << "I/O Error: A input line does not start with an int... exiting...\n";
         exit(EXIT_FAILURE);
      }
      else
      {
         vector<int> line_data;
         line_data.push_back(value);
         while(iss >> value) {
           line_data.push_back(value);
         }

         if (line_data.size() == 0)
            continue;

         //lines terminating with -1
         if(*(--line_data.end()) == -1)
         {
            if (line_data.size() > 1)
            {
              Graph c; 
              c.v_id = line_data.at(0);
              Graph::vertices.push_back(c);
     
              for(unsigned int i=1; i<line_data.size(); ++i)
              {
                int e = line_data.at(i);
                if (e == -1) break;
                while ((int)Graph::nets.size() < e) {
                   Graph::nets.push_back(vector<int>());
                }
                Graph::nets[e-1].push_back(c.v_id);
              }
            } else { //-1 line
              break;
            }
         }
         else //fixed cell line(s)
         {
            cerr << "I/O Error: Each line must terminate with -1; " << f_name << "\n"; 
            exit(EXIT_FAILURE);
         }
      }
   }
   if (in_file.bad()) {
      cerr << "I/O Error: There was a problem(s) with reading the file - " << f_name << "\n"; 
      exit(EXIT_FAILURE);
   }
   in_file.close();
   cout << "Okay: finished parsing the input file : " << f_name << "\n"; 
}

void act_on_mouse_move (float x, float y) {
	// function to handle mouse move event, the current mouse position in the current world coordinate
	// system (as defined in your call to init_world) is returned
	std::cout << "Mouse move at " << x << "," << y << ")\n";
}

//keep cnt of how many nodes are drawn on each level
Tree * root_node;

void begin_graphics (Tree* root)
{
   t_bound_box initial_coords = t_bound_box(0,0,1000,1000);

   init_graphics("Branch and Bound - Progression View", WHITE);
   set_visible_world(initial_coords);

   std::ostringstream str_buf;
   str_buf << Graph::vertices.size() << " vertices connected with " << Graph::nets.size() << " nets";
   std::string disp_str = str_buf.str();
   update_message(disp_str);

   root_node = root;
   event_loop(NULL, act_on_mouse_move, NULL, drawtree);   

   close_graphics ();
   std::cout << "Graphics closed down.\n";
}

const double row_width = 1000.0;
const double height = 1000.0;
void drawtree (void)
{
   double level_step = height / Graph::vertices.size();
   set_draw_mode(DRAW_NORMAL);
   setlinestyle (SOLID);
   setcolor (MAGENTA);
   setlinewidth (1);

   clearscreen();

   std::list<std::tuple<Tree*, int, double>> queue;
   queue.push_back(std::tuple<Tree*, int, double>(root_node, 0, 500));

   while(!queue.empty())
   {
     Tree* node = std::get<0>(queue.front());
     int level = std::get<1>(queue.front());
     double x_pos = std::get<2>(queue.front());

     queue.pop_front();
     double step_width = row_width;
     for(int i=0; i<level; i++) {
        step_width /= 2;
     }
     double init_offset = step_width / 2;

     if (node != nullptr) {
        t_point node_pt = t_point(x_pos, height-(level*level_step));
        t_bound_box node_rect = t_bound_box(node_pt, 1.5, 1.5);

        if (node->partition[node->node_idx] == Partition::L_ASSIGNED)
          setcolor(RED);
        else
          setcolor(BLUE);
        fillrect(node_rect);

        setcolor (MAGENTA);
        if(node->left_node != nullptr && node->right_node != nullptr) {
          drawline (node_pt.x, node_pt.y, node_pt.x - init_offset, node_pt.y - level_step);
          queue.push_back(std::tuple<Tree*, int, double>(node->left_node, level+1, x_pos - init_offset));
          drawline (node_pt.x, node_pt.y, node_pt.x + init_offset, node_pt.y - level_step);
          queue.push_back(std::tuple<Tree*, int, double>(node->right_node, level+1, x_pos + init_offset));
        } else if (node->left_node != nullptr) {
          drawline (node_pt.x, node_pt.y, node_pt.x, node_pt.y - level_step);
          queue.push_back(std::tuple<Tree*, int, double>(node->left_node, level+1, x_pos));
        } else if (node->right_node != nullptr) {
          drawline (node_pt.x, node_pt.y, node_pt.x, node_pt.y - level_step);
          queue.push_back(std::tuple<Tree*, int, double>(node->right_node, level+1, x_pos));
        }
     }
   }
}
