#include "utility.h"

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

const float c_cell_width = 15;

void begin_graphics (void) {
	int  grid_dim = g_fpga_grid[0].size();
   float   screen_dim         = c_cell_width * g_fpga_grid.at(0).size();
   t_bound_box initial_coords = t_bound_box(0,0,screen_dim,screen_dim); 

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
   //----------------------
   // Draw Grid Rectangles
   //----------------------
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

   float c_track_gap = c_cell_width / (GridCell::s_ch_width + 1); 
   t_point row_marker = t_point(0,0);
   t_bound_box cell_rect = t_bound_box(row_marker, c_cell_width, c_cell_width);
   setlinestyle (SOLID);
   setlinewidth (2);

   //DRAW text on LB
   for (auto r_it = g_fpga_grid.begin(); r_it != g_fpga_grid.end(); ++r_it) {
   	for (auto c_it = r_it->begin(); c_it != r_it->end(); ++c_it) {
   		if (c_it->m_type == CellType::SWITCH_BOX) {
   			setcolor (WHITE);
   			fillrect(cell_rect);
   		}
   		else if (c_it->m_type == CellType::LOGIC_BLOCK) {
   			setcolor (LIGHTGREY);
   			fillrect(cell_rect);
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
   	row_marker += t_point (0,c_cell_width);
   	cell_rect   = t_bound_box(row_marker, c_cell_width, c_cell_width);
   }
   
   //----------------------
   // Draw Nets
   //----------------------
   int color_idx = 5;
   for (auto it = g_fpga_nets.begin(); it != g_fpga_nets.end(); ++it) {
      if (it->m_routed) {
         //If net is routed, follow its path graph to draw
         int path_idx = 0;
         t_point last_point;
         setcolor(color_indicies[color_idx]);
         for (auto i = it->m_graph.begin(); i != it->m_graph.end(); ++i) {
             t_point c_bot_left = t_point(c_cell_width*((*i)->m_x_pos), c_cell_width*((*i)->m_y_pos));
             t_point src_point;
             t_point tgt_point;
             int hops = 0;
             if ((*i)->m_type == CellType::LOGIC_BLOCK) {
                int tgt_pin = it->o_pins.at(path_idx+1) % GridCell::s_ch_width;
                //DRAW LB->x_CH connection
                if (i == it->m_graph.begin()) {
                   //move point to o_pin location
                   switch (it->o_pins.at(path_idx)) {
                      case SOUTH:
                        hops = GridCell::s_ch_width - 1 - tgt_pin;
                        src_point = c_bot_left + t_point(c_cell_width / 2, 0); 
                        tgt_point = src_point;
                        tgt_point.y = tgt_point.y - c_track_gap;
                        tgt_point.y = tgt_point.y - (c_track_gap * hops);
                        break;
                      case EAST:
                        hops = tgt_pin;
                        src_point = c_bot_left + t_point(c_cell_width, c_cell_width / 2); 
                        tgt_point = src_point;
                        tgt_point.x = tgt_point.x + c_track_gap;
                        tgt_point.x = tgt_point.x + (c_track_gap * hops);
                        break;
                      case NORTH:
                        hops = tgt_pin;
                        src_point = c_bot_left + t_point(c_cell_width / 2, c_cell_width); 
                        tgt_point = src_point;
                        tgt_point.y = tgt_point.y + c_track_gap;
                        tgt_point.y = tgt_point.y + (c_track_gap * hops);
                        break;
                      case WEST:
                        hops = GridCell::s_ch_width - 1 - tgt_pin;
                        src_point = c_bot_left + t_point(0, c_cell_width / 2); 
                        tgt_point.x = src_point.x - c_track_gap;
                        tgt_point.x = tgt_point.x - (c_track_gap * hops);
                        break;
                   }
                } else { //CH->LB

                }
		          setlinestyle (SOLID);
             } else if((*i)->m_type == CellType::V_CHANNEL) { 
                //move x position to the right exit side (E, W)
                int side_idx  = it->o_pins.at(path_idx) / GridCell::s_ch_width;
                src_point = last_point;
                tgt_point.y = last_point.y;

                //place tgt_point on the output pin
                switch (side_idx) {
                   case 0: // EAST
                     tgt_point.x = c_bot_left.x + c_cell_width;
                     break;
                   case 1: // WEST
                     tgt_point.x = c_bot_left.x;
                     break;
                }
		          setlinestyle (SOLID);
             } else if((*i)->m_type == CellType::H_CHANNEL) { 
                //move y position to the right exit side (S, N)
                int side_idx  = it->o_pins.at(path_idx) / GridCell::s_ch_width;
                src_point = last_point;
                tgt_point.x = last_point.x;

                //place tgt_point on the output pin
                switch (side_idx) {
                   case 0: // SOUTH
                     tgt_point.y = c_bot_left.y;
                     break;
                   case 1: // NORTH
                     tgt_point.y = c_bot_left.y + c_cell_width;
                     break;
                }
		          setlinestyle (SOLID);
             } else if((*i)->m_type == CellType::SWITCH_BOX) { 
                int side_idx  = it->o_pins.at(path_idx) / GridCell::s_ch_width;
                hops = it->o_pins.at(path_idx) % GridCell::s_ch_width; //horizontal

                //connect last drawn point to the SB pinout
                src_point = last_point;
                tgt_point = c_bot_left;

                if (side_idx > 3) {
                   std::cerr << "CRITICAL ERROR: too many pins allocated to the cell!\n";
                }
                int grid_dim = g_fpga_grid.at(0).size();
					 //conpensate side_idx for SB orientation
					 if ((*i)->m_y_pos == 0) { //on bottom row
					 	if ((*i)->m_x_pos == 0) { //leftmost col (null, E, N, null)
					 	  //SW corner
                    side_idx++; 
					 	} else if ((*i)->m_x_pos == grid_dim-1) { //rightmost col (null, null, N, W)
                    side_idx += 2; 
					 	} else { //south, middle cols (null, E, N, W)
                    side_idx++; 
					 	}
					 } else if ((*i)->m_y_pos == grid_dim-1) { //on top row
					 	if ((*i)->m_x_pos == 0) { //leftmost col (S, E, null, null)
                    //no-op
					 	} else if ((*i)->m_x_pos == grid_dim-1) { //rightmost col (S, null, null, W)
                    if (side_idx == 1) //W
                        side_idx += 2; 
					 	} else { //north, middle cols (S, E, null, W)
                    if (side_idx == 2) //W
                        side_idx++; 
					 	}
					 } else { //middle rows
					 	if ((*i)->m_x_pos == 0) { //leftmost col (S, E, N, null)
                    //no-op
					 	} else if ((*i)->m_x_pos == grid_dim-1) { //rightmost col (S, null, N, W)
                    if (side_idx == 1 || side_idx == 2) //N, W
                        side_idx++; 
					 	} else { //middle cols (all 4)
                    //no-op
					 	}
					 } //end middle rows

                //place tgt_point on the output pin (start from bottom left)
                switch (side_idx) {
                   case SOUTH: //hop horizontally to output pin
                     tgt_point.x = tgt_point.x + c_track_gap;
                     tgt_point.x = tgt_point.x + (c_track_gap * hops);
                     break;
                   case EAST:  //jump east, hop veritcally
                     tgt_point.x = tgt_point.x + c_cell_width;
                     tgt_point.y = tgt_point.y + c_track_gap;
                     tgt_point.y = tgt_point.y + (c_track_gap * hops);
                     break;
                   case NORTH: //jump north, hop horizontally
                     tgt_point.y = tgt_point.y + c_cell_width;
                     tgt_point.x = tgt_point.x + c_track_gap;
                     tgt_point.x = tgt_point.y + (c_track_gap * hops);
                     break;
                  case WEST: //hop vertically to output pin
                     tgt_point.y = tgt_point.y + c_track_gap;
                     tgt_point.y = tgt_point.y + (c_track_gap * hops);
                     break;
                }
		          setlinestyle (DASHED);
             }

             //draw src_point to tgt_point
             drawline(src_point, tgt_point);

             //update values for next hop 
             path_idx += 1;
             last_point = tgt_point;
         }
      }

      //TODO: update color index 
      color_idx++;
      if(color_idx == 10) {
         color_idx = 4;
      }
   }
}
