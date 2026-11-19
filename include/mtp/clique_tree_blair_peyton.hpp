#pragma once
#include "mtp/clique_tree.hpp"
#include <vector>

namespace mtp {
class Graph;
// Blair & Peyton (1993), Section 4.2.2, Figure 4.2.
// The MCS selection is supplied by maximum_cardinality_search(G).
// For disconnected graphs, apply Ibarra (2008), Section 2 dummy links.
// O(n+m) time for a valid MCS-derived PEO of a chordal graph.
[[nodiscard]] CliqueTree build_clique_tree_blair_peyton(
    const Graph& graph, const std::vector<int>& mcs_peo);
} // namespace mtp
