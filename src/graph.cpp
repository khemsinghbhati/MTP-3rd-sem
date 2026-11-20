#include "mtp/graph.hpp"

#include <stdexcept>
#include <algorithm>

namespace mtp {

Graph Graph::from_edges(
        int n,const std::vector<std::pair<int,int>>& edges) {
    Graph graph(n);
    // Bucket by neighbor ID, not by source. Iterating buckets
    // in increasing order supplies each source's sorted neighbors
    // without an O(m log n) comparison sort.
    std::vector<std::vector<int>> by_neighbor(static_cast<std::size_t>(n));
    for(const auto& [u,v]:edges) {
        graph.validate_vertex(u);
        graph.validate_vertex(v);
        if(u==v)throw std::invalid_argument("self-loops are not allowed");
        by_neighbor[v].push_back(u);
        by_neighbor[u].push_back(v);
    }
    for(int v=0;v<n;++v) {
        for(int u:by_neighbor[v]) {
            auto& adj=graph.adjacency_[u];
            if(!adj.empty() && adj.back()==v)
                throw std::invalid_argument("duplicate input edge");
            adj.push_back(v);
        }
    }
    for(int v=0;v<n;++v)
        graph.neighbor_sets_[v]=std::set<int>(
            graph.adjacency_[v].begin(),graph.adjacency_[v].end());
    graph.edge_count_=edges.size();
    return graph;
}

Graph::Graph(int vertex_count) {
    if (vertex_count < 0) {
        throw std::invalid_argument("vertex_count must be nonnegative");
    }
    adjacency_.resize(static_cast<std::size_t>(vertex_count));
    neighbor_sets_.resize(static_cast<std::size_t>(vertex_count));
}

int Graph::vertex_count() const noexcept {
    return static_cast<int>(adjacency_.size());
}

std::size_t Graph::edge_count() const noexcept { return edge_count_; }

void Graph::validate_vertex(int vertex) const {
    if (vertex < 0 || vertex >= vertex_count()) {
        throw std::out_of_range("vertex ID outside [0, vertex_count)");
    }
}

const std::vector<int>& Graph::neighbors(int vertex) const {
    validate_vertex(vertex);
    return adjacency_[static_cast<std::size_t>(vertex)];
}

bool Graph::has_edge(int u, int v) const {
    validate_vertex(u);
    validate_vertex(v);
    return neighbor_sets_[static_cast<std::size_t>(u)].count(v) != 0;
}

void Graph::add_edge(int u, int v) {
    validate_vertex(u);
    validate_vertex(v);
    if (u == v) {
        throw std::invalid_argument("self-loops are not allowed");
    }
    if (neighbor_sets_[static_cast<std::size_t>(u)].insert(v).second) {
        neighbor_sets_[static_cast<std::size_t>(v)].insert(u);
        adjacency_[static_cast<std::size_t>(u)].push_back(v);
        adjacency_[static_cast<std::size_t>(v)].push_back(u);
        ++edge_count_;
    }
}

void Graph::remove_edge(int u, int v) {
    validate_vertex(u);
    validate_vertex(v);
    if (u == v) throw std::invalid_argument("self-loops are not allowed");
    if (neighbor_sets_[static_cast<std::size_t>(u)].erase(v) == 0) return;
    neighbor_sets_[static_cast<std::size_t>(v)].erase(u);
    auto& first = adjacency_[static_cast<std::size_t>(u)];
    auto& second = adjacency_[static_cast<std::size_t>(v)];
    first.erase(std::remove(first.begin(), first.end(), v), first.end());
    second.erase(std::remove(second.begin(), second.end(), u), second.end());
    --edge_count_;
}

} // namespace mtp
