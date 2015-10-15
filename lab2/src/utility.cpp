#include "utility.h"

const float c_cell_width = 15;

void begin_graphics (void) {
   t_bound_box initial_coords = t_bound_box(0,0,1000,1000); 

   init_graphics("Analytical Placer ", WHITE);
   set_visible_world(initial_coords);

   //std::ostringstream str_buf;
   //str_buf << grid_dim << " x " << grid_dim << " Grid";
   //std::string disp_str = str_buf.str();
   //update_message(disp_str);
   //event_loop(NULL, NULL, NULL, drawscreen);   
   //t_bound_box old_coords = get_visible_world();
}

