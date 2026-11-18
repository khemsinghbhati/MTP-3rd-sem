#include "mtp/clique_graph.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace mtp {

int clique_intersection_size(const std::vector<int>& a,
                             const std::vector<int>& b) {
    std::size_t i = 0, j = 0;
    int result = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            ++result;
            ++i;
            ++j;
        } else if (a[i] < b[j]) {
            ++i;
        } else {
            ++j;
        }
    }
    return result;
}

WeightedCliqueGraph build_weighted_clique_graph(
    const std::vector<std::vector<int>>& maximal_cliques) {
    WeightedCliqueGraph graph;
    graph.cliques = maximal_cliques;
    for (auto& clique : graph.cliques) {
        if (clique.empty()) {
            throw std::invalid_argument("maximal clique must not be empty");
        }
        std::sort(clique.begin(), clique.end());
        if (clique.front() < 0 ||
            std::adjacent_find(clique.begin(), clique.end()) != clique.end()) {
            throw std::invalid_argument("invalid or duplicated vertex in clique");
        }
    }
    const int k = static_cast<int>(graph.cliques.size());
    for (int i = 0; i < k; ++i) {
        for (int j = i + 1; j < k; ++j) {
            const int weight = clique_intersection_size(graph.cliques[i], graph.cliques[j]);
            if (weight > 0) graph.edges.push_back(CliqueEdge{i, j, weight});
        }
    }
    return graph;
}

} // namespace mtp
