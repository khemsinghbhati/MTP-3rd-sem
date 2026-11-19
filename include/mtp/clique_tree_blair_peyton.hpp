#pragma once
#include "mtp/clique_tree.hpp"
#include <vector>
namespace mtp {
class Graph;
// Blair & Peyton (1993), Figure 4.2; uses an MCS-produced PEO.
// O(n+m) for graph already represented by adjacency lists.
CliqueTree build_clique_tree_blair_peyton(
    const Graph& graph, const std::vector<int>& peo);
} // namespace mtp
