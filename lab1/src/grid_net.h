#include <vector>
#include <cmath>
#include <list>

using namespace std;

class GridNet {
        struct PathTree {
           GridCell          *node_ptr;
           vector<PathTree*> children;     
           int               level;
           int               level;
        };
 
        //Source to Target
        int m_source_x, m_source_y, source_pin;
        int m_target_x, m_target_y, target_pin;

        //Coarse-Routed Graph (i.e. List)
        vector<Vertex> m_cr_graph;

        //Detail-Routed Graph (i.e Tree)
        //------------------------------
 

    public:
        int s_net_id;
        int s_n_groups; //allowable sub-tree groups
        int s_k_groups; //

        GridNet();
        ~GridNet();

        int getLineDistance();
};

GridNet() {
}
//return linear distance 
int GridNet::getLineDistance() {
    return sqrt(((pow(abs(m_target_x - m_source_x)), 2) + pow(abs(m_target_y - m_source_y), 2)));
}


