#pragma once
#include "mtp/dynamic_ibarra.hpp"
#include <utility>
#include <vector>

namespace mtp {

// QUERY-OPTIMIZED REBUILD VARIANT — NOT the Sleator–Tarjan worst-case
// dynamic-tree implementation required by Ibarra (2008), Section 5.
//
// Source methods:
// - Ibarra (2008), Section 5, closest clique-node/shortest path query;
// - Ibarra (2008), Section 5.1, O(n) intersection-weight updates;
// - Sleator and Tarjan (1983), Section 5, heavy/light size partition.
//
// The heavy-path index is rebuilt in O(n) after each tree update rather than
// dynamically maintained with O(log n) worst-case links/cuts. Whole graph
// operations meet the per-operation update/query targets for this variant,
// but its dynamic-tree primitive is NOT the source-specified ST structure.
class IndexedDynamicChordalGraph {
public:
    explicit IndexedDynamicChordalGraph(Graph graph);
    [[nodiscard]] bool insert_query(int u, int v) const; // O(log^2 n)
    [[nodiscard]] bool delete_query(int u, int v) const; // O(n)
    bool insert_edge(int u, int v); // O(n)
    bool delete_edge(int u, int v); // O(n), within O(n log n) target
    void enable_fast_delete_queries();
    [[nodiscard]] CliqueTree snapshot() const;
    [[nodiscard]] const Graph& graph() const noexcept;

private:
    // Rooted heavy-path indexing of one static clique tree. All index arrays
    // use persistent clique node IDs from DynamicChordalGraph.
    struct HeavyPathIndex {
        std::vector<int> parent, depth, heavy, head, position, by_position;
        std::vector<int> subtree_size, parent_weight, segment;
        int base = 1;
        static constexpr int infinity = 1000000000;
        void build(const DynamicChordalGraph& data);
        [[nodiscard]] int lca(int a, int b) const;
        [[nodiscard]] int ancestor(int v, int distance) const;
        [[nodiscard]] int kth(int a, int b, int k) const;
        [[nodiscard]] int path_length(int a, int b) const;
        [[nodiscard]] int min_on_path(int a, int b) const;
        [[nodiscard]] int segment_min(int first, int last) const;
    };

    DynamicChordalGraph data_;
    HeavyPathIndex paths_;
    std::vector<int> vertex_witness_;
    std::vector<std::vector<int>> intersection_weight_;
    [[nodiscard]] std::pair<int,int> closest(int u, int v) const;
    void initialize_weights();
    void refresh_paths();
};

} // namespace mtp
