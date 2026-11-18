#pragma once

#include <vector>

namespace mtp {

class Graph;

// Returns the reverse of the MCS selection order. For chordal graphs this is a
// perfect elimination ordering (PEO); for general graphs this is NOT guaranteed.
// Time: O(V+E) on an undirected simple graph.
[[nodiscard]] std::vector<int> maximum_cardinality_search(const Graph& graph);

// Checks whether 'ordering' is a PEO of the given simple graph.
// Returns false for invalid orderings or a non-PEO.
// Worst-case O(V+E) time using grouped parent-neighbor marking.
[[nodiscard]] bool is_perfect_elimination_ordering(
    const Graph& graph, const std::vector<int>& ordering);

} // namespace mtp
