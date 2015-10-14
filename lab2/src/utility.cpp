#include "utility.h"

const float c_cell_width = 15;

void begin_graphics (void) {
   int     grid_dim = g_fpga_grid[0].size();
   float   screen_dim         = c_cell_width * g_fpga_grid.at(0).size();
   t_bound_box initial_coords = t_bound_box(0,0,screen_dim,screen_dim); 

   init_graphics("Analytical Place ", WHITE);
   set_visible_world(initial_coords);

   //std::ostringstream str_buf;
   //str_buf << grid_dim << " x " << grid_dim << " Grid";
   //std::string disp_str = str_buf.str();
   update_message(disp_str);

   event_loop(NULL, NULL, NULL, drawscreen);   
   //t_bound_box old_coords = get_visible_world();
}

