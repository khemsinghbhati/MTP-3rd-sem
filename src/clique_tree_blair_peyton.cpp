#include "mtp/clique_tree_blair_peyton.hpp"
#include "mtp/graph.hpp"
#include "mtp/mcs.hpp"

#include <stdexcept>
#include <vector>

namespace mtp {
CliqueTree build_clique_tree_blair_peyton(
        const Graph& graph, const std::vector<int>& mcs_peo) {
    if (!is_perfect_elimination_ordering(graph, mcs_peo)) {
        throw std::invalid_argument("Blair-Peyton requires an MCS-derived valid PEO");
    }
    const int n = graph.vertex_count();
    CliqueTree tree;
    if (n == 0) return tree;

    std::vector<int> position(n), clique_of_vertex(n, -1);
    for (int i = 0; i < n; ++i) position[mcs_peo[i]] = i;
    int previous_card = 0;
    int first_component_clique = -1;

    // Blair & Peyton Fig. 4.2, i = n,...,1. MCS vertex selection
    // is performed by the caller before passing its PEO.
    for (int index = n - 1; index >= 0; --index) {
        const int u = mcs_peo[index];
        int current_card = 0;
        int minimum_labeled_neighbor = -1;
        for (int v : graph.neighbors(u)) {
            if (position[v] <= index) continue; // previously numbered
            ++current_card;
            if (minimum_labeled_neighbor < 0 ||
                position[v] < position[minimum_labeled_neighbor]) {
                minimum_labeled_neighbor = v;
            }
        }
        // Fig. 4.2: begin a new clique iff new_card <= prev_card.
        if (current_card <= previous_card) {
            const int new_clique = static_cast<int>(tree.cliques.size());
            tree.cliques.emplace_back();
            // K_s <- adj(v_i) intersect L_{i+1}.
            for (int v : graph.neighbors(u)) {
                if (position[v] > index) tree.cliques.back().push_back(v);
            }
            if (current_card != 0) {
                // k <- min{j : v_j in K_s}; p <- clique(v_k).
                const int parent_clique = clique_of_vertex[minimum_labeled_neighbor];
                if (parent_clique < 0 || parent_clique == new_clique)
                    throw std::logic_error("Blair-Peyton parent clique unavailable");
                tree.edges.push_back({new_clique, parent_clique, current_card, false});
            } else {
                // Ibarra Section 2: dummy edge joining components, weight 0.
                if (first_component_clique < 0)
                    first_component_clique = new_clique;
                else
                    tree.edges.push_back({new_clique, first_component_clique, 0, true});
            }
        }
        const int s = static_cast<int>(tree.cliques.size()) - 1;
        clique_of_vertex[u] = s;               // clique(v_i) <- s
        tree.cliques.back().push_back(u);       // K_s <- K_s union {v_i}
        previous_card = current_card;
    }
    if (tree.edges.size() + 1 != tree.cliques.size())
        throw std::logic_error("Blair-Peyton invalid clique-tree edge count");
    return tree;
}
} // namespace mtp
