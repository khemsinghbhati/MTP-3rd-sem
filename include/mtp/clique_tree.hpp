#pragma once

#include "mtp/clique_graph.hpp"

#include <vector>

namespace mtp {

// A clique-tree edge. A dummy edge has weight zero and connects two
// different connected components of the original graph.
struct CliqueTreeEdge {
    int first;
    int second;
    int weight;
    bool dummy;
};

struct CliqueTree {
    std::vector<std::vector<int>> cliques;
    std::vector<CliqueTreeEdge> edges;
};

// Constructs an MW spanning forest of W_G, then joins its components with
// arbitrary weight-zero dummy edges (Ibarra 2008, Section 2, p. 40:6).
// A chordal graph's output satisfies the running-intersection property.
// Time: O(|E(W_G)| log |E(W_G)| + k alpha(k)), plus clique copies.
[[nodiscard]] CliqueTree build_clique_tree(const WeightedCliqueGraph& graph);

// Independent invariant checker: tree structure, accurate edge weights,
// component-wise dummy edges, and induced subtree for every original vertex.
// Does not substitute for independent maximal-clique validation.
[[nodiscard]] bool verify_clique_tree(const WeightedCliqueGraph& graph,
                                       const CliqueTree& tree);

} // namespace mtp
