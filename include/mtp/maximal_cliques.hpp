#pragma once

#include <vector>

namespace mtp {

class Graph;

// Enumerates maximal cliques from a valid perfect elimination ordering.
// Throws invalid_argument if ordering is not a PEO of graph.
// O(V+E+S) expected time, where S is the number of listed vertex occurrences.
[[nodiscard]] std::vector<std::vector<int>> extract_maximal_cliques(
    const Graph& graph, const std::vector<int>& peo);

} // namespace mtp
