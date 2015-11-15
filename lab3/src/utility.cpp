#include "utility.h"

//*****************************************************************************
//* sort cartesian points (v_id, x_pos, y_pos) across x dim, and then by y dim 
//* dividing them across 4 quadrants
//*****************************************************************************
//void sort_by_k(std::vector<std::tuple<int, double, double>> & points)
//{
//   std::sort(points.begin(), points.end(), sort_by_x());
//   auto m_it = points.begin() + (points.size() / 2);
//   std::sort(points.begin(), m_it, sort_by_y());
//   std::sort(m_it, points.end(), sort_by_y());
//   return;
//}

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

void begin_graphics (void)
{
   t_bound_box initial_coords = t_bound_box(0,0,100,100);

   init_graphics("Branch and Bound - Progression View", WHITE);
   set_visible_world(initial_coords);

   std::ostringstream str_buf;
   str_buf << Graph::vertices.size() << " vertices connected with " << Graph::nets.size() << " nets";
   std::string disp_str = str_buf.str();
   update_message(disp_str);

   
   event_loop(NULL, NULL, NULL, drawscreen);   
   //if(!created_button) {
   //  create_button ("Window", "Toggle Lines", act_on_toggle_nets_button); // name is UTF-8
   //  created_button = true;
   //}
   t_bound_box old_coords = get_visible_world(); //save the current view for later
}

void drawscreen (void)
{
   set_draw_mode(DRAW_NORMAL);
   clearscreen();
}
