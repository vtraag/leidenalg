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

    void get_memberships(size_t node);
    void add_membership(size_t node, size_t cover, double weight);

    /**
     * Create aggregate cover based on partition.
     *
     * @param partition Partition on basis of which to aggregate
     * @return Aggregated partition
     */
    VertexCover* create_aggregate_cover(MutableVertexPartition& partition);

private:

    vector< vector< pair<size_t, double> > > membership; // (Weighted) membership of each node
};

#endif //LEIDENALG_TEST_VERTEXCOVER_H
