#ifndef VERTEXCOVER_H
#define VERTEXCOVER_H

#include <vector>
#include "MutableVertexPartition.h"

using std::vector;
using std::pair;

class VertexCover {

public:
    VertexCover(vector<size_t>& membership);
    VertexCover(vector< vector< pair<size_t, double> > >& membership);

    vector< pair<size_t, double> > get_memberships(size_t node);

    double get_membership(size_t node, size_t comm);

    VertexCover* collapse_cover(MutableVertexPartition* partition);

private:

    size_t _cached_node;
    vector<double> _cached_node_membership;
    vector< vector< pair<size_t, double> > > _membership; // (Weighted) membership of each node

    void cache_membership(size_t node);
    void set_cache(size_t node);
    void reset_cache();
    void init_cache();

    size_t _max_cover;
};

#endif //LEIDENALG_TEST_VERTEXCOVER_H
