#include "LinearResolutionParameterVertexPartition.h"

LinearResolutionParameterVertexPartition::LinearResolutionParameterVertexPartition(Graph* graph,
      vector<size_t> membership, double resolution_parameter) :
        MutableVertexPartition(graph,
        membership)
{ this->resolution_parameter = resolution_parameter; }

LinearResolutionParameterVertexPartition::LinearResolutionParameterVertexPartition(Graph* graph,
      vector<size_t> membership) :
        MutableVertexPartition(graph,
        membership)
{ this->resolution_parameter = 1.0; }

LinearResolutionParameterVertexPartition::LinearResolutionParameterVertexPartition(Graph* graph,
  double resolution_parameter) :
        MutableVertexPartition(graph)
{ this->resolution_parameter = resolution_parameter;  }

LinearResolutionParameterVertexPartition::LinearResolutionParameterVertexPartition(Graph* graph) :
        MutableVertexPartition(graph)
{ this->resolution_parameter = 1.0;  }

LinearResolutionParameterVertexPartition::~LinearResolutionParameterVertexPartition()
{ }
