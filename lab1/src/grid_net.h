#include <vector>
#include <cmath>
#include <list>

using namespace std;

struct Coordinate {
	int x;
	int y;
	int p;
	Coordinate() : x(0), y(0), p(0) {}
	Coordinate(int _x, int _y, , int _p) : x(_x), y(_y), p(_p) {}
	bool operator==(const Coordinate &lhs, const Coordinate &rhs) {
		if (lhs.x == rhs.x && lhs.y == rhs.y && lhs.p == rhs.p)
			return true;
		else 
			return false;
	}
};

class GridNet {
	struct CrTree {
		GridCell          *node_ptr;
		vector<PathTree*> children;     
	   int               path_cost;
	};
	int m_line_dist;

	//Detail-Routed Graph (i.e Tree)
	//------------------------------
	vector<PathTree> m_dr_graph;


	public:
	   //Source to Target
	   int m_src_x, m_src_y, m_src_p;
	   int m_tgt_x, m_tgt_y, m_tgt_p;
	   int m_tgt_row, m_target_col, m_target_pin;

	   //Coarse-Routed Graph (i.e. List)
	   list<GridCell*> m_cr_graph;

	   static int s_n_groups; //allowable sub-tree groups
	   static int s_k_param;  //

	   GridNet();
	   GridNet(int, int, int, int, int, int);
	   ~GridNet();

	   int        expand();
	   int        getLineDistance();
	   Coordinate getSrcCoordinate();
	   Coordinate getTgtCoordinate();
};


GridNet(int _s_x, int _s_y, int _s_p, int _t_x, int _t_y, int _t_p) : m_cr_graph(), m_dr_graph() {
	m_src_x = _s_x;
	m_src_y = _s_y;
	m_src_p = _s_p;

	m_tgt_x = _t_x;
	m_tgt_y = _t_y;
	m_tgt_p = _t_p;

	m_line_dist = sqrt(((pow(abs(m_tgt_x - m_src_x)), 2) + pow(abs(m_tgt_y - m_src_y), 2)));
}

int GridNet::getLineDistance() {
	return m_line_dist;
}

Coordinate GridNet::getSrcCoordinate() {
	Coordinate retval(m_src_x, m_src_y, m_src_p);
	return retval;
}

Coordinate GridNet::getTgtCoordinate() {
	Coordinate retval(m_tgt_x, m_tgt_y, m_tgt_p);
	return retval;
}

int GridNet::expand() {
   if ()
   return EXIT_FAILURE;
}


