#include "mtp/clique_tree.hpp"

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mtp {
namespace {

class DisjointSet {
public:
    explicit DisjointSet(int n) : parent_(static_cast<std::size_t>(n)),
                                  rank_(static_cast<std::size_t>(n), 0) {
        for (int i = 0; i < n; ++i) parent_[i] = i;
    }

    int find(int x) {
        if (parent_[x] != x) parent_[x] = find(parent_[x]);
        return parent_[x];
    }

    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (rank_[a] < rank_[b]) std::swap(a, b);
        parent_[b] = a;
        if (rank_[a] == rank_[b]) ++rank_[a];
        return true;
    }

private:
    std::vector<int> parent_;
    std::vector<int> rank_;
};

} // namespace

CliqueTree build_clique_tree(const WeightedCliqueGraph& graph) {
    CliqueTree result;
    result.cliques = graph.cliques;
    const int k = static_cast<int>(graph.cliques.size());
    if (k == 0) return result;

    std::vector<CliqueEdge> edges = graph.edges;
    std::sort(edges.begin(), edges.end(), [](const CliqueEdge& a, const CliqueEdge& b) {
        if (a.weight != b.weight) return a.weight > b.weight;
        if (a.first != b.first) return a.first < b.first;
        return a.second < b.second;
    });

    DisjointSet connected(k);
    for (const auto& e : edges) {
        if (connected.unite(e.first, e.second)) {
            result.edges.push_back({e.first, e.second, e.weight, false});
        }
    }
    // Every remaining component corresponds to a disconnected component of G.
    // A dummy edge is not an edge of the original W_G.
    for (int i = 1; i < k; ++i) {
        if (connected.unite(0, i)) {
            result.edges.push_back({0, i, 0, true});
        }
    }
    return result;
}

bool verify_clique_tree(const WeightedCliqueGraph& graph, const CliqueTree& tree) {
    const int k = static_cast<int>(graph.cliques.size());
    if (tree.cliques != graph.cliques) return false;
    if (tree.edges.size() != static_cast<std::size_t>(k == 0 ? 0 : k - 1)) return false;

    DisjointSet tree_components(k);
    DisjointSet positive_components(k);
    for (const auto& e : graph.edges) {
        if (e.first < 0 || e.second < 0 || e.first >= k || e.second >= k ||
            e.first == e.second || e.weight <= 0 ||
            e.weight != clique_intersection_size(graph.cliques[e.first],
                                                 graph.cliques[e.second])) {
            return false;
        }
        positive_components.unite(e.first, e.second);
    }

    std::vector<std::vector<int>> adjacency(static_cast<std::size_t>(k));
    for (const auto& e : tree.edges) {
        if (e.first < 0 || e.second < 0 || e.first >= k || e.second >= k ||
            e.first == e.second || !tree_components.unite(e.first, e.second)) {
            return false;
        }
        const int actual_weight = clique_intersection_size(tree.cliques[e.first],
                                                            tree.cliques[e.second]);
        if (actual_weight != e.weight || e.dummy != (actual_weight == 0)) return false;
        if (e.dummy && positive_components.find(e.first) ==
                           positive_components.find(e.second)) return false;
        adjacency[e.first].push_back(e.second);
        adjacency[e.second].push_back(e.first);
    }

    if (k > 0) {
        for (int i = 1; i < k; ++i) {
            if (tree_components.find(i) != tree_components.find(0)) return false;
        }
    }

    std::unordered_map<int, std::vector<int>> containing;
    for (int i = 0; i < k; ++i) {
        for (int v : tree.cliques[i]) containing[v].push_back(i);
    }
    // Induced-subtree property: all clique nodes containing each original
    // vertex must be connected using only nodes also containing that vertex.
    for (const auto& [vertex, cliques] : containing) {
        std::vector<bool> belongs(static_cast<std::size_t>(k), false);
        for (int c : cliques) belongs[c] = true;
        std::vector<bool> visited(static_cast<std::size_t>(k), false);
        std::queue<int> q;
        q.push(cliques.front());
        visited[cliques.front()] = true;
        std::size_t reached = 0;
        while (!q.empty()) {
            const int c = q.front();
            q.pop();
            ++reached;
            for (int next : adjacency[c]) {
                if (belongs[next] && !visited[next]) {
                    visited[next] = true;
                    q.push(next);
                }
            }
        }
        if (reached != cliques.size()) return false;
        (void)vertex;
    }
    return true;
}

} // namespace mtp
