#include <vector>
#include <cmath>
#include <list>

using namespace std;

class GridNet {
        struct Vertex {
           GridCell *v_ptr;
        };
 
        //Source to Target
        int m_source_x, m_source_y, source_pin;
        int m_target_x, m_target_y, target_pin;

        //Coarse-Routed Graph (i.e. List)
        vector<Vertex> m_cr_graph;

        //Detail-Routed Graph (i.e Tree)
        //------------------------------
 

    public:
        GridNet();
        ~GridNet();

        int getLinearDistance();
};

//return linear distance 
int GridNet::getLinearDistance() {
    return sqrt(((pow(abs(m_target_x - m_source_x)), 2) + pow(abs(m_target_y - m_source_y), 2)));
}

