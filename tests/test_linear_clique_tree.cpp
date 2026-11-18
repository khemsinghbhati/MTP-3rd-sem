#include "mtp/clique_tree_linear.hpp"
#include "mtp/graph.hpp"
#include "mtp/maximal_cliques.hpp"
#include "mtp/mcs.hpp"

#include <algorithm>
#include <iostream>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {
using namespace mtp;
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
std::vector<std::vector<int>> canonical(std::vector<std::vector<int>> data) {
    for (auto& clique : data) std::sort(clique.begin(), clique.end());
    std::sort(data.begin(), data.end());
    return data;
}
int intersection(const std::vector<int>& a, const std::vector<int>& b) {
    int result = 0;
    for (int v : a) {
        if (std::find(b.begin(), b.end(), v) != b.end()) ++result;
    }
    return result;
}
void check(const Graph& graph, bool compare_mwst = true) {
    const auto peo = maximum_cardinality_search(graph);
    if (!is_perfect_elimination_ordering(graph, peo)) return;
    const auto tree = build_clique_tree_linear(graph, peo);
    require(canonical(tree.cliques) ==
                canonical(extract_maximal_cliques(graph, peo)),
            "direct construction does not enumerate the correct cliques");
    const int k = static_cast<int>(tree.cliques.size());
    require(static_cast<int>(tree.edges.size()) == (k ? k - 1 : 0),
            "clique tree has wrong edge count");
    std::vector<std::vector<int>> adj(k);
    int total_weight = 0;
    for (const auto& edge : tree.edges) {
        require(edge.first >= 0 && edge.second >= 0 &&
                edge.first < k && edge.second < k &&
                edge.first != edge.second, "invalid clique-tree edge");
        const int weight = intersection(tree.cliques[edge.first],
                                        tree.cliques[edge.second]);
        require(weight == edge.weight, "incorrect separator weight");
        require(edge.dummy == (weight == 0), "incorrect dummy-edge flag");
        adj[edge.first].push_back(edge.second);
        adj[edge.second].push_back(edge.first);
        total_weight += weight;
    }
    if (k) {
        std::queue<int> q;
        std::vector<bool> seen(k, false);
        q.push(0); seen[0] = true;
        int count = 0;
        while (!q.empty()) {
            const int u = q.front(); q.pop(); ++count;
            for (int v : adj[u]) {
                if (!seen[v]) { seen[v] = true; q.push(v); }
            }
        }
        require(count == k, "clique tree is disconnected");
    }
    // The cliques containing each original vertex must induce a subtree.
    for (int vertex = 0; vertex < graph.vertex_count(); ++vertex) {
        std::vector<bool> member(k, false), seen(k, false);
        int first = -1, expected = 0;
        for (int c = 0; c < k; ++c) {
            if (std::find(tree.cliques[c].begin(), tree.cliques[c].end(),
                          vertex) == tree.cliques[c].end()) continue;
            member[c] = true; first = c; ++expected;
        }
        require(expected > 0, "vertex missing from all maximal cliques");
        std::queue<int> q;
        q.push(first); seen[first] = true;
        int reached = 0;
        while (!q.empty()) {
            const int u = q.front(); q.pop(); ++reached;
            for (int v : adj[u]) {
                if (member[v] && !seen[v]) {
                    seen[v] = true; q.push(v);
                }
            }
        }
        require(reached == expected, "running-intersection property failed");
    }
    if (compare_mwst && k <= 12) {
        // Independent baseline: maximum spanning tree on every clique pair.
        struct Edge { int u, v, weight; };
        std::vector<Edge> edges;
        for (int i = 0; i < k; ++i)
            for (int j = i + 1; j < k; ++j)
                edges.push_back({i, j,
                    intersection(tree.cliques[i], tree.cliques[j])});
        std::sort(edges.begin(), edges.end(),
                  [](const Edge& a, const Edge& b) {
                      return a.weight > b.weight;
                  });
        std::vector<int> parent(k);
        for (int i = 0; i < k; ++i) parent[i] = i;
        auto root = [&](int x) {
            while (parent[x] != x) x = parent[x] = parent[parent[x]];
            return x;
        };
        int best = 0;
        for (const auto& edge : edges) {
            int u = root(edge.u), v = root(edge.v);
            if (u != v) { parent[u] = v; best += edge.weight; }
        }
        require(total_weight == best, "maximum-weight spanning tree mismatch");
    }
}
void targeted() {
    Graph sample(7);
    for (const auto& [u, v] : std::vector<std::pair<int, int>>{
        {0,1}, {1,3}, {0,2}, {2,3}, {2,4},
        {2,5}, {4,5}, {3,6}, {0,3}, {1,2}
    }) sample.add_edge(u,v);
    check(sample);
    const auto tree = build_clique_tree_linear(
        sample, maximum_cardinality_search(sample));
    require(tree.cliques.size() == 3 && tree.edges.size() == 2,
            "sample did not yield a three-node clique tree");
    check(Graph(0)); check(Graph(1)); check(Graph(8));
    Graph disconnected(6);
    disconnected.add_edge(0,1);
    disconnected.add_edge(2,3);
    check(disconnected);
    // Large sparse input with a dense explicit clique intersection graph.
    Graph star(2000);
    for (int i = 1; i < 2000; ++i) star.add_edge(0,i);
    check(star,false);
    Graph complete(400);
    for (int u = 0; u < 400; ++u)
        for (int v = u + 1; v < 400; ++v)
            complete.add_edge(u,v);
    check(complete,false);
}
void exhaustive() {
    int chordal = 0;
    for (int n = 1; n <= 6; ++n) {
        std::vector<std::pair<int,int>> possible;
        for (int u = 0; u < n; ++u)
            for (int v = u+1; v < n; ++v)
                possible.push_back({u,v});
        for (int mask = 0; mask < (1 << static_cast<int>(possible.size())); ++mask) {
            Graph graph(n);
            for (int j = 0; j < static_cast<int>(possible.size()); ++j)
                if (mask & (1 << j)) graph.add_edge(possible[j].first,
                                                    possible[j].second);
            const auto peo = maximum_cardinality_search(graph);
            if (is_perfect_elimination_ordering(graph,peo)) {
                ++chordal; check(graph);
            }
        }
    }
    require(chordal == 19048, "unexpected exhaustive chordal-graph count");
}
void randomized() {
    std::mt19937 rng(20261118);
    for (int trial = 0; trial < 800; ++trial) {
        const int n = 7 + static_cast<int>(rng()%7);
        Graph graph(0);
        for (int size = 0; size < n; ++size) {
            Graph next(size+1);
            for (int u = 0; u < size; ++u)
                for (int v : graph.neighbors(u))
                    if (u<v) next.add_edge(u,v);
            if (size > 0) {
                const auto options = extract_maximal_cliques(
                    graph, maximum_cardinality_search(graph));
                const auto& clique=options[rng()%options.size()];
                for (int v : clique) if (rng()%3) next.add_edge(size,v);
            }
            graph = std::move(next);
        }
        check(graph);
    }
}
} // namespace

int main() {
    try {
        targeted(); exhaustive(); randomized();
        std::cout << "PASS: direct clique tree construction\n"
                  << "PASS: 19,048 exhaustive chordal graphs on n<=6\n"
                  << "PASS: 800 randomized chordal graphs, large star and clique\n";
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
