#include "mtp/graph.hpp"
#include "mtp/maximal_cliques.hpp"
#include "mtp/mcs.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

using Cliques = std::vector<std::vector<int>>;

Cliques canonical(Cliques cliques) {
    for (auto& clique : cliques) std::sort(clique.begin(), clique.end());
    std::sort(cliques.begin(), cliques.end());
    return cliques;
}

// Independent brute-force oracle, for small n <= 12.
Cliques brute_maximal_cliques(const mtp::Graph& graph) {
    const int n = graph.vertex_count();
    assert(n <= 12);
    Cliques maximal;
    for (int mask = 1; mask < (1 << n); ++mask) {
        bool clique = true;
        for (int u = 0; u < n && clique; ++u) {
            if ((mask & (1 << u)) == 0) continue;
            for (int v = u + 1; v < n; ++v) {
                if ((mask & (1 << v)) && !graph.has_edge(u, v)) {
                    clique = false;
                    break;
                }
            }
        }
        if (!clique) continue;
        bool is_maximal = true;
        for (int u = 0; u < n && is_maximal; ++u) {
            if (mask & (1 << u)) continue;
            bool extendible = true;
            for (int v = 0; v < n; ++v) {
                if ((mask & (1 << v)) && !graph.has_edge(u, v)) {
                    extendible = false;
                    break;
                }
            }
            if (extendible) is_maximal = false;
        }
        if (!is_maximal) continue;
        std::vector<int> members;
        for (int v = 0; v < n; ++v) if (mask & (1 << v)) members.push_back(v);
        maximal.push_back(std::move(members));
    }
    return canonical(std::move(maximal));
}

// Independent chordality oracle using simplicial-vertex elimination DP.
// For small graphs, a graph is chordal iff repeatedly eliminating some
// simplicial vertex can eliminate all vertices.
bool brute_chordal(const mtp::Graph& graph) {
    const int n = graph.vertex_count();
    assert(n <= 12);
    const int full_mask = (1 << n) - 1;
    std::vector<int> memo(static_cast<std::size_t>(1 << n), -1);
    auto solve = [&](auto&& self, int mask) -> bool {
        if (mask == 0) return true;
        int& answer = memo[static_cast<std::size_t>(mask)];
        if (answer != -1) return answer == 1;
        for (int u = 0; u < n; ++u) {
            if (!(mask & (1 << u))) continue;
            bool simplicial = true;
            for (int v = 0; v < n && simplicial; ++v) {
                if (v == u || !(mask & (1 << v)) || !graph.has_edge(u, v)) continue;
                for (int w = v + 1; w < n; ++w) {
                    if (w == u || !(mask & (1 << w))) continue;
                    if (graph.has_edge(u, w) && !graph.has_edge(v, w)) {
                        simplicial = false;
                        break;
                    }
                }
            }
            if (simplicial && self(self, mask ^ (1 << u))) {
                answer = 1;
                return true;
            }
        }
        answer = 0;
        return false;
    };
    return solve(solve, full_mask);
}

void check(const mtp::Graph& graph, bool expected_chordal) {
    const auto order = mtp::maximum_cardinality_search(graph);
    const bool is_peo = mtp::is_perfect_elimination_ordering(graph, order);
    if (is_peo != expected_chordal) {
        throw std::runtime_error("MCS / PEO verification disagrees with chordality oracle");
    }
    if (!is_peo) {
        try {
            (void)mtp::extract_maximal_cliques(graph, order);
            throw std::runtime_error("nonchordal extraction should have thrown");
        } catch (const std::invalid_argument&) {
            return;
        }
    }
    const auto actual = canonical(mtp::extract_maximal_cliques(graph, order));
    const auto expected = brute_maximal_cliques(graph);
    if (actual != expected) {
        throw std::runtime_error("maximal clique enumeration disagrees with brute force");
    }
}

void sample_tests() {
    mtp::Graph graph(7);
    for (const auto& [u, v] : std::vector<std::pair<int, int>>{
             {0, 1}, {1, 3}, {0, 2}, {2, 3}, {2, 4},
             {2, 5}, {4, 5}, {3, 6}, {0, 3}, {1, 2}}) {
        graph.add_edge(u, v);
    }
    const auto order = mtp::maximum_cardinality_search(graph);
    const std::vector<int> expected_order{4, 5, 1, 2, 0, 3, 6};
    if (order != expected_order) throw std::runtime_error("midsem PEO changed");
    const Cliques expected{{0, 1, 2, 3}, {2, 4, 5}, {3, 6}};
    if (canonical(mtp::extract_maximal_cliques(graph, order)) != canonical(expected)) {
        throw std::runtime_error("midsem sample cliques changed");
    }

    graph.add_edge(0, 1);  // duplicate ignored
    if (graph.edge_count() != 10) throw std::runtime_error("duplicate edge changed size");

    bool rejected_loop = false;
    try { graph.add_edge(1, 1); } catch (const std::invalid_argument&) { rejected_loop = true; }
    if (!rejected_loop) throw std::runtime_error("self-loop was not rejected");

    check(mtp::Graph(0), true);
    check(mtp::Graph(1), true);
    mtp::Graph disconnected(4);
    check(disconnected, true);

    mtp::Graph cycle(4);
    for (int i = 0; i < 4; ++i) cycle.add_edge(i, (i+1)%4);
    check(cycle, false);
}

int exhaustive_tests() {
    int chordal_count = 0;
    for (int n = 1; n <= 6; ++n) {
        std::vector<std::pair<int, int>> possible;
        for (int u = 0; u < n; ++u) {
            for (int v = u+1; v < n; ++v) possible.emplace_back(u, v);
        }
        const int graph_count = 1 << static_cast<int>(possible.size());
        for (int mask = 0; mask < graph_count; ++mask) {
            mtp::Graph graph(n);
            for (int j = 0; j < static_cast<int>(possible.size()); ++j) {
                if (mask & (1 << j)) graph.add_edge(possible[static_cast<std::size_t>(j)].first,
                                                    possible[static_cast<std::size_t>(j)].second);
            }
            const bool chordal = brute_chordal(graph);
            if (chordal) ++chordal_count;
            check(graph, chordal);
        }
    }
    return chordal_count;
}

void randomized_chordal_tests() {
    std::mt19937 gen(20261118);  // fixed seed for deterministic CI
    for (int trial = 0; trial < 600; ++trial) {
        const int n = 7 + static_cast<int>(gen() % 6); // 7..12
        mtp::Graph graph(0);
        // Construct chordal graphs by repeatedly adding a simplicial vertex
        // whose earlier neighbors are a subset of an existing maximal clique.
        for (int size = 0; size < n; ++size) {
            mtp::Graph expanded(size + 1);
            for (int u = 0; u < size; ++u) {
                for (int v : graph.neighbors(u)) if (u < v) expanded.add_edge(u, v);
            }
            if (size > 0) {
                const auto existing_peo = mtp::maximum_cardinality_search(graph);
                const auto available = mtp::extract_maximal_cliques(graph, existing_peo);
                const auto& clique = available[static_cast<std::size_t>(gen() % available.size())];
                for (int v : clique) if (gen() % 3 != 0) expanded.add_edge(size, v);
            }
            graph = std::move(expanded);
        }
        check(graph, true);
    }
}

} // namespace

int main() {
    try {
        sample_tests();
        const int chordal_count = exhaustive_tests();
        randomized_chordal_tests();
        std::cout << "PASS: supplied sample and edge cases\n"
                  << "PASS: all simple graphs on 1..6 vertices ("
                  << chordal_count << " chordal graphs, exhaustive)\n"
                  << "PASS: 600 randomized chordal graphs on 7..12 vertices\n";
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
