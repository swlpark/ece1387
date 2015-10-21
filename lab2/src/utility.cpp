#include "utility.h"

struct sort_by_x {
    bool operator()(const std::tuple<int,double,double> &left, const std::tuple<int,double,double> &right) {
        return std::get<1>(left) < std::get<1>(right);
    }
};

struct sort_by_y {
    bool operator()(const std::tuple<int,double,double> &left, const std::tuple<int,double,double> &right) {
        return std::get<2>(left) < std::get<2>(right);
    }
};

std::vector<double> solveQ(std::vector<std::vector<double>> const &cols, std::vector<Vertex> const & fixed_cells, std::vector<Vertex> const * virtual_cells)
{
  //assert N x N dim of cols matrix
  assert(cols.size() == cols.at(0).size());

  int m_dim = cols.size();
  std::vector<double> retval;
  retval.resize(2*m_dim);

  double * X  = new double[m_dim]();
  double * Y  = new double[m_dim]();
  double * Bx = new double[m_dim]();
  double * By = new double[m_dim]();

  //construct rhs vectors - Bx, By
  for (unsigned int i=0; i < fixed_cells.size(); ++i)
  {
      std::list<Edge> const & adj_cells = fixed_cells.at(i).adj_list;
      for(auto it = adj_cells.begin(); it != adj_cells.end(); ++it)
      {
         if ((*it).tgt->fixed)
           continue;
         int idx = Vertex::v_map_table.at((*it).tgt->v_id - 1);
         assert(idx < m_dim && idx >= 0);
         Bx[idx] += (*it).weight * (double)fixed_cells.at(i).x_pos;
         By[idx] += (*it).weight * (double)fixed_cells.at(i).y_pos;
      }
  }
  if (virtual_cells != nullptr)
  {
     for (unsigned int i=0; i < (*virtual_cells).size(); ++i)
     {
         std::list<Edge> const & adj_cells = (*virtual_cells).at(i).adj_list;
         for(auto it = adj_cells.begin(); it != adj_cells.end(); ++it)
         {
            if ((*it).tgt->fixed)
              continue;
            int idx = Vertex::v_map_table.at((*it).tgt->v_id - 1);
            assert(idx < m_dim && idx >= 0);
            Bx[idx] += (*it).weight * (double)(*virtual_cells).at(i).x_pos;
            By[idx] += (*it).weight * (double)(*virtual_cells).at(i).y_pos;
         }
     }
  }


  //UMFPACK sparse matrix solver arguments
  int *    Ap = new int[m_dim+1];
  int *    Ai;
  double * Ax;

  Ap[0] = 0;
  int nz_cnt = 0;
  for (int c=0; c<m_dim; ++c)
  {
    for(int r=0; r<m_dim; ++r)
    {
       if (cols[c][r] != 0)
          ++nz_cnt;
    }
    Ap[c+1] = nz_cnt;
  }

  Ai = new int[nz_cnt];
  Ax = new double[nz_cnt];

  int idx = 0;
  for (int c=0; c<m_dim; ++c)
  {
    for(int r=0; r<m_dim; ++r)
    {
       if (cols[c][r] != 0)
       { 
          Ai[idx] = r;
          Ax[idx] = cols[c][r];
          ++idx;
       }
    }
  }

#ifdef _DEBUG_
  std::cout << "Bx: ";
  for(int i =0; i<m_dim; ++i)
  {
    std::cout << Bx[i] << " ";
  }
  std::cout << "\n";
  std::cout << "By: ";
  for(int i =0; i<m_dim; ++i)
  {
    std::cout << Bx[i] << " ";
  }
  std::cout << "\n";
  std::cout << "Ap: ";
  for(int i =0; i<m_dim+1; ++i)
  {
    std::cout << Ap[i] << " ";
  }
  std::cout << "\n";
  std::cout << "Ai: ";
  for(int i =0; i<nz_cnt; ++i)
  {
    std::cout << Ai[i] << " ";
  }
  std::cout << "\n";
  std::cout << "Ax: ";
  for(int i =0; i<nz_cnt; ++i)
  {
    std::cout << Ax[i] << " ";
  }
  std::cout << "\n";
#endif

  double *null = (double *) NULL;
  void   *Symbolic, *Numeric;

  (void) umfpack_di_symbolic (m_dim, m_dim, Ap, Ai, Ax, &Symbolic, null, null);
  (void) umfpack_di_numeric (Ap, Ai, Ax, Symbolic, &Numeric, null, null);
  umfpack_di_free_symbolic(&Symbolic);

  (void) umfpack_di_solve (UMFPACK_A, Ap, Ai, Ax, X, Bx, Numeric, null, null);
  (void) umfpack_di_solve (UMFPACK_A, Ap, Ai, Ax, Y, By, Numeric, null, null);
  umfpack_di_free_numeric(&Numeric);

  //push X, Y into return vector
  for(int i=0; i<m_dim; ++i)
  {
    retval[i] = X[i];
    retval[i+m_dim] = Y[i];
  }

  delete [] X;
  delete [] Y;
  delete [] Bx;
  delete [] By;
  delete [] Ap;
  delete [] Ai;
  delete [] Ax;

  return retval;
}

//*****************************************************************************
//* sort cartesian points (v_id, x_pos, y_pos) across x dim, and then by y dim 
//* dividing them across 4 quadrants
//*****************************************************************************
void partition_quadrants(std::vector<std::tuple<int, double, double>> & points)
{
   std::sort(points.begin(), points.end(), sort_by_x());

   auto m_it = points.begin() + (points.size() / 2);

   std::sort(points.begin(), m_it, sort_by_y());
   std::sort(m_it, points.end(), sort_by_y());

   return;
}

void begin_graphics (void)
{
   t_bound_box initial_coords = t_bound_box(0,0,100,100);

   init_graphics("Analytical Placer", WHITE);
   set_visible_world(initial_coords);

   std::ostringstream str_buf;
   str_buf << cells.size() << " cells placed with " << nets.size() << " nets";
   std::string disp_str = str_buf.str();
   update_message(disp_str);
   event_loop(NULL, NULL, NULL, drawscreen);   
   //t_bound_box old_coords = get_visible_world();
}

void drawscreen (void)
{
   clearscreen();

   for(auto it = cells.begin(); it != cells.end(); ++it)
   {
      t_point bt_marker = t_point(it->x_pos - 0.3, it->y_pos - 0.3);
      t_bound_box cell_rect = t_bound_box(bt_marker, 0.6, 0.6);
      if (it->fixed)
        setcolor(RED);
      else
        setcolor(BLUE);
      fillrect(cell_rect);
   }

   setcolor(BLACK);
   for(auto it = virtual_pins.begin(); it != virtual_pins.end(); ++it)
   {
      t_point bt_marker = t_point(it->x_pos - 0.3, it->y_pos - 0.3);
      t_bound_box cell_rect = t_bound_box(bt_marker, 0.6, 0.6);
      fillrect(cell_rect);
   }

   setcolor(MEDIUMPURPLE);
   setlinestyle(SOLID);
   setlinewidth(1);
   std::vector<int> q_to_c_map;
   q_to_c_map.resize(Q.size());

   unsigned int q_idx=0;
   for(unsigned int i=0; i<cells.size(); i++)
   {
      if (Vertex::v_map_table[i] == -1)
        continue;
      assert(q_idx < Q.size());
      q_to_c_map[q_idx++] = i;
   }

   int drawn_lines =0;

   //draw lines between movable cells
   for(unsigned int c=0; c<Q.size(); c++)
   {
      for(unsigned int r=c+1; r<Q.size(); r++)
      {
         if (Q[c][r] != 0)
         {
             int src_idx = q_to_c_map.at(c);
             int tgt_idx = q_to_c_map.at(r);
             drawline(cells[src_idx].x_pos, cells[src_idx].y_pos, 
                      cells[tgt_idx].x_pos, cells[tgt_idx].y_pos);
             drawn_lines++;
         }
      }
   }

   setcolor(RED);
   setlinestyle(DASHED);
   setlinewidth(1);
   //used edge set 
   std::unordered_set<std::pair<int, int>> u_edges;

   for(auto f_iter = fixed_cells.begin();  f_iter != fixed_cells.end(); ++f_iter)
   {
     std::list<Edge>& adj_cells = f_iter->adj_list;

     //iterating over the edge list to draw
     for(auto t_iter = adj_cells.begin(); t_iter != adj_cells.end(); ++t_iter)
     {
        std::pair<int, int> edge (f_iter->v_id, t_iter->tgt->v_id);
        auto set_idx = u_edges.find(edge);

        //skip if edge is found in the set
        if (set_idx != u_edges.end()) {
           //std::cout << "DEBUG: set found a duplicate edge, src_cell=" << f_iter->v_id  <<"\n";
           continue;
        }
        u_edges.insert(edge);
        drawline(f_iter->x_pos, f_iter->y_pos, 
                 t_iter->tgt->x_pos, t_iter->tgt->y_pos);
        drawn_lines++;
     }
   }

   std::cout << "Number of lines drawn: " << drawn_lines << "\n";
}

