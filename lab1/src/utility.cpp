#include "utility.h"

constexpr float screen_dim = 1000;

static t_bound_box initial_coords = t_bound_box(0,0,screen_dim,screen_dim); 

void build_FPGA_grid (std::vector<std::vector<GridCell>> &grid, int grid_dim) {
	grid.reserve(grid_dim);

	for (int i=0; i < grid_dim; ++i) {
      grid.push_back (std::vector<GridCell>());
		grid[i].reserve(grid_dim);
		for (int j=0; j< grid_dim; ++j) { 
			GridCell cell(j, i); //i.e. cell(col, row) 
			grid[i].push_back(cell);
		}
	}

	for (int i=0; i < grid_dim; ++i) {
		for (int j=0; j < grid_dim; ++j) { 
			if(i % 2) { //odd row
				if (j % 2) {//odd col
					//LBs...
					grid[i][j].setAdjacency(&grid[i-1][j], &grid[i][j+1],
							&grid[i+1][j], &grid[i][j-1]);
				} else { //even col => HCs...
               if(j == 0) {//leftmost col, connects to right LB
					  grid[i][j].setAdjacency(&grid[i-1][j], &grid[i][j+1],
					  		&grid[i+1][j], nullptr);
               } else if (j == grid_dim-1) { //rightmost col, connects to left LB
					  grid[i][j].setAdjacency(&grid[i-1][j], nullptr,
					  		&grid[i+1][j], &grid[i][j-1]);
               } else { //inner cols //connects to left, right LBs
					  grid[i][j].setAdjacency(&grid[i-1][j], &grid[i][j+1],
					  		&grid[i+1][j], &grid[i][j-1]);
               }
				}
			} else { //even row
				if (j % 2) { //odd col
					//VCs...
					if (i == 0) { //bottom row
					  grid[i][j].setAdjacency(nullptr, &grid[i][j+1],
					  		&grid[i+1][j], &grid[i][j-1]);
               } else if (i==grid_dim-1) { //top row
					  grid[i][j].setAdjacency(&grid[i-1][j], &grid[i][j+1],
					  		nullptr, &grid[i][j-1]);
               } else { //middle rows
					  grid[i][j].setAdjacency(&grid[i-1][j], &grid[i][j+1],
					  		&grid[i+1][j], &grid[i][j-1]);
               }
				} else { //even col
					//SBs...
					if (i == 0) { //on bottom row
						if (j == 0) { //leftmost col
							//SW corner
							grid[i][j].setAdjacency(nullptr, &grid[i][j+1],
									&grid[i+1][j], nullptr);
						} else if (j == grid_dim-1) { //rightmost col
							//SE corner
							grid[i][j].setAdjacency(nullptr, nullptr,
									&grid[i+1][j], &grid[i][j-1]);

						} else { //south, middle cols
							grid[i][j].setAdjacency(nullptr, &grid[i][j+1],
									&grid[i+1][j], &grid[i][j-1]);
						}
					} else if (i==grid_dim-1) { //on top row
						if (j == 0) { //leftmost col
							//NW corner
							grid[i][j].setAdjacency(&grid[i-1][j], &grid[i][j+1],
									nullptr, nullptr);
						} else if (j == grid_dim-1) { //rightmost col
							//NE corner
							grid[i][j].setAdjacency(&grid[i-1][j], nullptr,
									nullptr, &grid[i][j-1]);

						} else { //north, middle cols
							grid[i][j].setAdjacency(&grid[i-1][j], &grid[i][j+1],
									nullptr, &grid[i][j-1]);
						}
					} else { //middle rows
						if (j == 0) { //leftmost col
							grid[i][j].setAdjacency(&grid[i-1][j], &grid[i][j+1],
									&grid[i+1][j], nullptr);
						} else if (j == grid_dim-1) { //rightmost col
							grid[i][j].setAdjacency(&grid[i-1][j], nullptr,
									&grid[i+1][j], &grid[i][j-1]);
						} else { //middle cols
							grid[i][j].setAdjacency(&grid[i-1][j], &grid[i][j+1],
									&grid[i+1][j], &grid[i][j-1]);
						}
					} //end middle rows
				} //end even col
			} //end even row
		} //end col loop
	} //end row loop
}

void print_FPGA_grid (std::vector<std::vector<GridCell>> &grid) {
  int row_cnt =0;
  for (auto r_it = grid.begin(); r_it != grid.end(); ++r_it) {
        std::cout << "ROW-" << row_cnt++ << ": ";
     for (auto c_it = r_it->begin(); c_it != r_it->end(); ++c_it) {
        std::string desc;
        switch ((*c_it).m_type) {
          case CellType::LOGIC_BLOCK :
             desc = std::string("LB");
             break;
          case CellType::V_CHANNEL :
             desc = std::string("VC");
             break;
          case CellType::H_CHANNEL :
             desc = std::string("HC");
             break;
          case CellType::SWITCH_BOX :
             desc = std::string("SB");
             break;
        }
        std::cout << desc << "(" << (*c_it).m_adj_cnt << ") ";
     }
        std::cout << "\n";
  }
}

std::string tostring_cell_type (GridCell * cell) {
    std::string desc;
        switch ((*cell).m_type) {
          case CellType::LOGIC_BLOCK :
             desc = std::string("LB");
             break;
          case CellType::V_CHANNEL :
             desc = std::string("VC");
             break;
          case CellType::H_CHANNEL :
             desc = std::string("HC");
             break;
          case CellType::SWITCH_BOX :
             desc = std::string("SB");
             break;
        }
    return desc;
}

/*TODO: implement uni-directional support
 * Return matching pin number on the target Cell
 */
int matchAdjacentPin (int src_o_pin, GridCell * source , GridCell * target) {
    int track_idx = src_o_pin % GridCell::s_ch_width;

    if (source->m_adj_south == target) {
      //TARGET_SIDE IS NORTH
      if (target->m_adj_south != nullptr) track_idx += GridCell::s_ch_width;
      if (target->m_adj_east != nullptr)  track_idx += GridCell::s_ch_width;

    } else if (source->m_adj_east == target) {
      //TARGET_SIDE IS WEST
      if (target->m_adj_south != nullptr) track_idx += GridCell::s_ch_width;
      if (target->m_adj_east != nullptr)  track_idx += GridCell::s_ch_width;
      if (target->m_adj_north != nullptr) track_idx += GridCell::s_ch_width;
    } else if (source->m_adj_north == target) {
      //TARGET_SIDE IS SOUTH
    } else if (source->m_adj_west == target) {
      //TARGET_SIDE IS EAST
      if (target->m_adj_south != nullptr) track_idx += GridCell::s_ch_width;
    } else {
      std::cerr << "matchAdjacentPin : could not find a matching cell... \n";
      return EXPAND_FAIL;
    }
    return track_idx;
}

void begin_graphics (void) {
	int  grid_dim = g_fpga_grid[0].size();
	init_graphics("FPGA Routing Grid", WHITE); // you could pass a t_color RGB triplet instead
	set_visible_world(initial_coords);

   std::ostringstream str_buf;
   str_buf << grid_dim << " x " << grid_dim << " Grid";
	std::string disp_str = str_buf.str();
	update_message(disp_str);

	event_loop(NULL, NULL, NULL, drawscreen);   
	t_bound_box old_coords = get_visible_world(); // save the current view for later;
}

void drawscreen (void) {
	color_types color_indicies[] = {
		LIGHTGREY,
		DARKGREY,
		WHITE,
		BLACK,
		BLUE,
		GREEN,
		YELLOW,
		CYAN,
		RED,
		DARKGREEN,
		MAGENTA
	};

	//----------------------
	// Draw Grid Rectangles
	//----------------------

	const float c_cell_width = screen_dim / g_fpga_grid[0].size();
	const float c_track_gap = c_cell_width / (GridCell::s_ch_width + 1); 
	//allow gaps between cell boundaries (+1)

	t_point row_marker = t_point(0,0);
	t_bound_box cell_rect = t_bound_box(row_marker, c_cell_width, c_cell_width);

	setlinestyle (SOLID);
	setlinewidth (2);

	for (auto r_it = g_fpga_grid.begin(); r_it != g_fpga_grid.end(); ++r_it) {
		for (auto c_it = r_it->begin(); c_it != r_it->end(); ++c_it) {
			if (c_it->m_type == CellType::SWITCH_BOX) {
				setcolor (WHITE);
				fillrect(cell_rect);
			}
			else if (c_it->m_type == CellType::LOGIC_BLOCK) {
				setcolor (LIGHTGREY);
				fillrect(cell_rect);

				//Draw PINS of LB
				//if(c_it->m_adj_south != nullptr) {
				//}
			}
			else if (c_it->m_type == CellType::H_CHANNEL) {
				//Draw vertical lines
				setcolor (BLACK);
				t_point marker = t_point(cell_rect.bottom_left().x, cell_rect.bottom_left().y);
				marker += t_point(c_track_gap, 0); //gap between first line and adj cell

				for(int i = 0; i < GridCell::s_ch_width; ++i) {
					drawline(marker.x, marker.y, marker.x, marker.y + c_cell_width);
					marker += t_point(c_track_gap, 0);
				}
			}
			else if (c_it->m_type == CellType::V_CHANNEL) {
				//Draw horizontal lines
				setcolor (BLACK);
				t_point marker = t_point(cell_rect.bottom_left().x, cell_rect.bottom_left().y);
				marker += t_point(0, c_track_gap); //gap between first line and adj cell

				for(int i = 0; i < GridCell::s_ch_width; ++i) {
					drawline(marker.x, marker.y, marker.x + c_cell_width, marker.y);
					marker += t_point(0, c_track_gap);
				}
			}
			cell_rect += t_point(c_cell_width,0);
		}
		row_marker += t_point(0,c_cell_width);
		cell_rect   = t_bound_box(row_marker, c_cell_width, c_cell_width);
	}
}
