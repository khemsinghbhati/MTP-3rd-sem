#pragma once

#include "mtp/clique_tree.hpp"
#include "mtp/fast_delete_ibarra.hpp"
#include "mtp/graph.hpp"

#include <list>
#include <memory>
#include <vector>

namespace mtp {

// Source-faithful Ibarra (2008) Sections 3-4 A1, with sparse clique
// incidence instead of eagerly initialized n-bit characteristic vectors.
//
// PRECONDITION: graph is already supplied as a Graph with n vertices and
// m edges (Graph::from_edges builds it in O(n+m) worst case).
// Constructor: O(n+m) worst case from MCS/Blair-Peyton and the
// clique-incidence bound sum_K |K| <= n+m for chordal graphs.
// Four operations: O(n) worst case, including deterministic adjacency lookup.
// Optional Ibarra A1+ index has its separately stated preprocessing/update
// costs. A2 retains its own dense legacy representation and is unaffected.
class SparseIbarraChordalGraphA1 {
public:
    explicit SparseIbarraChordalGraphA1(Graph graph);

    [[nodiscard]] bool delete_query(int u, int v) const;
    [[nodiscard]] bool insert_query(int u, int v) const;
    bool delete_edge(int u, int v);
    bool insert_edge(int u, int v);
    void enable_fast_delete_queries();

    [[nodiscard]] CliqueTree snapshot() const;
    [[nodiscard]] const Graph& graph() const noexcept { return graph_; }
    [[nodiscard]] int maximal_clique_count() const noexcept { return active_count_; }
    [[nodiscard]] std::size_t stored_memberships() const noexcept { return membership_count_; }

private:
    using InverseIterator = std::list<int>::iterator;
    struct Node {
        bool active = false;
        std::vector<int> members;
        std::vector<InverseIterator> inverse_iterators;
        std::list<int> incident;
    };
    struct Edge {
        bool active = false;
        int a=-1, b=-1, weight=0;
        std::list<int>::iterator in_a, in_b;
    };
    struct EndpointPair {
        int x=-1, y=-1;
        std::vector<int> path_edges; // edge IDs, from y backwards towards x
    };

    Graph graph_;
    std::vector<Node> nodes_;
    std::vector<Edge> edges_;
    std::vector<std::list<int>> nodes_of_; // inverse list T_v
    std::vector<int> free_nodes_, free_edges_;
    int active_count_=0;
    std::size_t membership_count_=0;
    std::unique_ptr<FastCliqueIntersectionIndex> fast_;

    int allocate_node(std::vector<int> vertices);
    void replace_members(int node, std::vector<int> vertices);
    void retire_node(int node);
    int link(int a, int b, int weight);
    void cut(int edge_id);
    void contract(int keep, int drop);
    [[nodiscard]] int clique_of_edge(int u, int v) const;
    [[nodiscard]] EndpointPair closest_pair(int u, int v) const;
    [[nodiscard]] int overlap(int a, int b) const;
    void add_to_fast_index(int node);
    void check_vertices(int u, int v) const;
};

} // namespace mtp
