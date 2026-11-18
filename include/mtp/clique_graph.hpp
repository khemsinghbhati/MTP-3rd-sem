#pragma once

#include <vector>

namespace mtp {

// An undirected edge of the weighted clique intersection graph W_G.
// first and second index maximal cliques, not vertices of the original graph.
struct CliqueEdge {
    int first;
    int second;
    int weight;
};

struct WeightedCliqueGraph {
    // Sorted member lists; original input clique order is preserved.
    std::vector<std::vector<int>> cliques;
    // Edges are listed only for NONEMPTY intersections (positive weights).
    std::vector<CliqueEdge> edges;
};

// Constructs the original weighted clique intersection graph W_G in Ibarra
// (2008), Section 2. Caller must supply ALL maximal cliques of a chordal graph.
// This routine validates individual clique lists but does not recompute
// maximality or chordality; those are established by the preceding stage.
// Time: O(sum |K_i| log |K_i| + sum_{i<j} (|K_i|+|K_j|)).
[[nodiscard]] WeightedCliqueGraph build_weighted_clique_graph(
    const std::vector<std::vector<int>>& maximal_cliques);

// Computes |A intersect B| when A and B are sorted ascending.
[[nodiscard]] int clique_intersection_size(const std::vector<int>& a,
                                           const std::vector<int>& b);

} // namespace mtp
