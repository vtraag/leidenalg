cmake_minimum_required(VERSION 3.17)
project(leidenalg)

set(CMAKE_CXX_STANDARD 14)

find_package(igraph REQUIRED PATHS $HOME)

include_directories(include)

add_library(leidenalg
        src/leidenalg/CPMVertexPartition.cpp
        src/leidenalg/GraphHelper.cpp
        src/leidenalg/LinearResolutionParameterVertexPartition.cpp
        src/leidenalg/ModularityVertexPartition.cpp
        src/leidenalg/MutableVertexPartition.cpp
        src/leidenalg/Optimiser.cpp
        src/leidenalg/RBConfigurationVertexPartition.cpp
        src/leidenalg/RBERVertexPartition.cpp
        src/leidenalg/ResolutionParameterVertexPartition.cpp
        src/leidenalg/SignificanceVertexPartition.cpp
        src/leidenalg/SurpriseVertexPartition.cpp)

target_link_libraries(leidenalg PUBLIC igraph::igraph)