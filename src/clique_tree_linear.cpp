#include "mtp/clique_tree_linear.hpp"

#include "mtp/graph.hpp"
#include "mtp/mcs.hpp"

#include <stdexcept>
#include <vector>

namespace mtp {

CliqueTree build_clique_tree_linear(const Graph& graph, const std::vector<int>& peo) {
    if (!is_perfect_elimination_ordering(graph, peo)) {
        throw std::invalid_argument("clique tree requires a valid PEO");
    }

    const int n = graph.vertex_count();
    CliqueTree tree;
    if (n == 0) return tree;

    std::vector<int> position(n), parent(n, -1), later_degree(n, 0);
    for (int i = 0; i < n; ++i) position[peo[i]] = i;

    // Elimination forest: the parent is the earliest later neighbor.
    for (int u = 0; u < n; ++u) {
        for (int v : graph.neighbors(u)) {
            if (position[v] <= position[u]) continue;
            ++later_degree[u];
            if (parent[u] == -1 || position[v] < position[parent[u]]) {
                parent[u] = v;
            }
        }
    }

    // A candidate C_p is contained in C_u when p=parent(u) and
    // |N+(u)| - 1 = |N+(p)|. Choose one child to absorb each such p.
    // All nodes without a witness represent maximal candidate cliques.
    std::vector<int> witness(n, -1);
    for (int u = 0; u < n; ++u) {
        const int p = parent[u];
        if (p != -1 && later_degree[u] - 1 == later_degree[p]) {
            witness[p] = u;
        }
    }

    // Keep the same maximal-clique numbering as extract_maximal_cliques().
    std::vector<int> representative_id(n, -1);
    for (int u : peo) {
        if (witness[u] != -1) continue;
        representative_id[u] = static_cast<int>(tree.cliques.size());
        std::vector<int> clique{u};
        for (int v : graph.neighbors(u)) {
            if (position[v] > position[u]) clique.push_back(v);
        }
        tree.cliques.push_back(std::move(clique));
    }

    // Contract selected witness edges of the elimination forest.
    // Their adjacency lists form a forest with O(n) edges. DFS assigns
    // the unique remaining maximal-clique ID to each component in O(n).
    std::vector<std::vector<int>> contracted(n);
    for (int p = 0; p < n; ++p) {
        if (witness[p] < 0) continue;
        const int u = witness[p];
        contracted[p].push_back(u);
        contracted[u].push_back(p);
    }

    std::vector<int> group(n, -1);
    std::vector<int> stack, members;
    stack.reserve(n);
    members.reserve(n);
    for (int start = 0; start < n; ++start) {
        if (group[start] != -1) continue;
        group[start] = -2;
        stack.push_back(start);
        members.clear();
        int clique_id = -1;
        while (!stack.empty()) {
            const int u = stack.back();
            stack.pop_back();
            members.push_back(u);
            if (representative_id[u] >= 0) {
                if (clique_id != -1) {
                    throw std::logic_error("contraction group has multiple maximal cliques");
                }
                clique_id = representative_id[u];
            }
            for (int v : contracted[u]) {
                if (group[v] == -1) {
                    group[v] = -2;
                    stack.push_back(v);
                }
            }
        }
        if (clique_id < 0) {
            throw std::logic_error("contraction group has no maximal clique");
        }
        for (int u : members) group[u] = clique_id;
    }

    // The separator of an uncontracted elimination edge (u,p(u)) is
    // N+(u), so its weight is the later degree of u. The running
    // intersection property preserves the weight after contraction.
    for (int u = 0; u < n; ++u) {
        const int p = parent[u];
        if (p >= 0 && group[u] != group[p]) {
            tree.edges.push_back({group[u], group[p], later_degree[u], false});
        }
    }

    // For a disconnected graph, connect the component clique trees
    // with zero-weight dummy edges (Ibarra 2008, Section 2).
    int first_root_group = -1;
    for (int u = 0; u < n; ++u) {
        if (parent[u] != -1) continue;
        if (first_root_group == -1) first_root_group = group[u];
        else tree.edges.push_back({first_root_group, group[u], 0, true});
    }

    if (tree.edges.size() + 1 != tree.cliques.size()) {
        throw std::logic_error("clique-tree edge count disagrees with node count");
    }
    return tree;
}

} // namespace mtp
