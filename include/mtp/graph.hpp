#pragma once

#include <cstddef>
#include <unordered_set>
#include <vector>

namespace mtp {

// Undirected, simple graph with zero-based vertex IDs [0, vertex_count).
// The adjacency vectors preserve insertion order (useful for reproducible MCS ties).
class Graph {
public:
    explicit Graph(int vertex_count);

    [[nodiscard]] int vertex_count() const noexcept;
    [[nodiscard]] std::size_t edge_count() const noexcept;
    [[nodiscard]] const std::vector<int>& neighbors(int vertex) const;
    [[nodiscard]] bool has_edge(int u, int v) const;

    // Rejects self-loops and out-of-range IDs. Repeated edges are ignored.
    void add_edge(int u, int v);
    // Delete an existing edge. O(n) worst case for adjacency-vector removal.
    void remove_edge(int u, int v);

private:
    void validate_vertex(int vertex) const;

    std::vector<std::vector<int>> adjacency_;
    std::vector<std::unordered_set<int>> neighbor_sets_;
    std::size_t edge_count_ = 0;
};

} // namespace mtp
