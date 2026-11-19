#pragma once
#include "mtp/graph.hpp"
#include "mtp/clique_tree.hpp"
#include "mtp/fast_delete_ibarra.hpp"
#include <memory>
#include <list>
#include <vector>

namespace mtp {
// Ibarra (2008), Sections 3-4, first fully dynamic chordal-graph algorithm.
// Maintains clique characteristic vectors and a clique tree under permitted
// updates. All four operations O(n) worst-case with O(1) linked-list link/cut.
class DynamicChordalGraph {
    friend class IndexedDynamicChordalGraph;
public:
    explicit DynamicChordalGraph(Graph graph);
    // Optional Ibarra Sections 6.2-6.3 accelerated deletion-query index.
    // Extra initialization O(mn+n^2); queries O(sqrt(m)+1), updates O(m+n).
    void enable_fast_delete_queries();
    [[nodiscard]] bool delete_query(int u, int v) const;
    [[nodiscard]] bool insert_query(int u, int v) const;
    bool delete_edge(int u, int v);
    bool insert_edge(int u, int v);
    [[nodiscard]] CliqueTree snapshot() const;
    [[nodiscard]] const Graph& graph() const noexcept { return graph_; }
    [[nodiscard]] int maximal_clique_count() const noexcept { return active_count_; }
private:
    struct Node {
        bool active = false;
        int size = 0;
        std::vector<unsigned char> contains;
        std::list<int> incident;
    };
    struct Edge {
        bool active = false;
        int a = -1, b = -1, weight = 0;
        std::list<int>::iterator in_a, in_b;
    };
    struct EndpointPair {
        int x = -1, y = -1;
        std::vector<int> path_edges;
    };
    Graph graph_;
    std::unique_ptr<FastCliqueIntersectionIndex> fast_;
    std::vector<Node> nodes_;
    std::vector<Edge> edges_;
    std::vector<int> free_nodes_, free_edges_;
    int active_count_ = 0;
    int allocate_node(std::vector<unsigned char> bits);
    int link(int a, int b, int weight);
    void cut(int id);
    void contract(int keep, int drop);
    int clique_of_edge(int u, int v) const;
    EndpointPair closest_pair(int u, int v) const;
    int overlap(int a, int b) const;
};
} // namespace mtp
