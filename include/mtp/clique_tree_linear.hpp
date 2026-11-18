#pragma once

#include "mtp/clique_tree.hpp"

#include <vector>

namespace mtp {
class Graph;

// Build a clique tree directly from a chordal graph and its valid PEO,
// without enumerating the potentially quadratic clique graph W_G.
// The elimination-forest contraction runs in deterministic O(n+m).
// This entry point validates the PEO with deterministic O(n+m) marking.
// Ibarra (2008) cites earlier O(n+m) clique-tree constructors; the precise
// contraction implementation here is NOT claimed to be verbatim from Ibarra.
[[nodiscard]] CliqueTree build_clique_tree_linear(
    const Graph& graph, const std::vector<int>& peo);

} // namespace mtp
