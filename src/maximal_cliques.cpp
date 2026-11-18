#include "mtp/maximal_cliques.hpp"

#include "mtp/graph.hpp"
#include "mtp/mcs.hpp"

#include <stdexcept>
#include <utility>
#include <vector>

namespace mtp {

std::vector<std::vector<int>> extract_maximal_cliques(
    const Graph& graph, const std::vector<int>& peo) {
    if (!is_perfect_elimination_ordering(graph, peo)) {
        throw std::invalid_argument("maximal clique extraction requires a valid PEO");
    }

    const int n = graph.vertex_count();
    std::vector<int> position(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) position[static_cast<std::size_t>(peo[static_cast<std::size_t>(i)])] = i;

    std::vector<std::vector<int>> later_neighbors(static_cast<std::size_t>(n));
    std::vector<int> parent(static_cast<std::size_t>(n), -1);
    std::vector<int> later_degree(static_cast<std::size_t>(n), 0);

    for (int u = 0; u < n; ++u) {
        for (const int v : graph.neighbors(u)) {
            if (position[static_cast<std::size_t>(v)] <= position[static_cast<std::size_t>(u)]) continue;
            later_neighbors[static_cast<std::size_t>(u)].push_back(v);
            if (parent[static_cast<std::size_t>(u)] == -1 ||
                position[static_cast<std::size_t>(v)] <
                    position[static_cast<std::size_t>(parent[static_cast<std::size_t>(u)])]) {
                parent[static_cast<std::size_t>(u)] = v;
            }
        }
        later_degree[static_cast<std::size_t>(u)] =
            static_cast<int>(later_neighbors[static_cast<std::size_t>(u)].size());
    }

    // Equivalent to the parent-subsumption test in the midsem code.
    std::vector<bool> maximal(static_cast<std::size_t>(n), true);
    for (int u = 0; u < n; ++u) {
        const int p = parent[static_cast<std::size_t>(u)];
        if (p != -1 && later_degree[static_cast<std::size_t>(u)] - 1 ==
                           later_degree[static_cast<std::size_t>(p)]) {
            maximal[static_cast<std::size_t>(p)] = false;
        }
    }

    std::vector<std::vector<int>> result;
    for (const int u : peo) {
        if (!maximal[static_cast<std::size_t>(u)]) continue;
        std::vector<int> clique;
        clique.reserve(1 + later_neighbors[static_cast<std::size_t>(u)].size());
        clique.push_back(u);
        const auto& tail = later_neighbors[static_cast<std::size_t>(u)];
        clique.insert(clique.end(), tail.begin(), tail.end());
        result.push_back(std::move(clique));
    }
    return result;
}

} // namespace mtp
