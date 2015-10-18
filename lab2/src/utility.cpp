#include "utility.h"

std::vector<double> solveQ(std::vector<std::vector<double>> const &cols, std::vector<Vertex> const & fixed_cells)
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

void begin_graphics (void) {
   t_bound_box initial_coords = t_bound_box(0,0,100,100); 

   init_graphics("Analytical Placer ", WHITE);
   set_visible_world(initial_coords);

   //std::ostringstream str_buf;
   //str_buf << grid_dim << " x " << grid_dim << " Grid";
   //std::string disp_str = str_buf.str();
   //update_message(disp_str);
   //event_loop(NULL, NULL, NULL, drawscreen);   
   //t_bound_box old_coords = get_visible_world();
}

