#include "mtp/dynamic_ibarra.hpp"
#include "mtp/clique_tree_blair_peyton.hpp"
#include "mtp/mcs.hpp"

#include <algorithm>
#include <queue>
#include <stdexcept>
#include <utility>

namespace mtp {
namespace {
int count_members(const std::vector<unsigned char>& bits) {
    int count = 0;
    for (unsigned char value : bits) count += static_cast<int>(value);
    return count;
}
} // namespace

DynamicChordalGraph::DynamicChordalGraph(Graph graph)
    : graph_(std::move(graph)) {
    const auto peo = maximum_cardinality_search(graph_);
    if (!is_perfect_elimination_ordering(graph_, peo))
        throw std::invalid_argument("initial graph must be chordal");
    const auto tree = build_clique_tree_blair_peyton(graph_, peo);
    const int n = graph_.vertex_count();
    nodes_.resize(static_cast<std::size_t>(n + 2));
    for (const auto& clique : tree.cliques) {
        std::vector<unsigned char> bits(n, 0);
        for (int vertex : clique) bits[vertex] = 1;
        allocate_node(std::move(bits));
    }
    for (const auto& edge : tree.edges)
        link(edge.first, edge.second, edge.weight);
}

void DynamicChordalGraph::enable_fast_delete_queries() {
    fast_ = std::make_unique<FastCliqueIntersectionIndex>(
        graph_.vertex_count(), static_cast<int>(graph_.edge_count()));
    for (int i=0;i<static_cast<int>(nodes_.size());++i)
        if (nodes_[i].active)
            fast_->add_clique(i,nodes_[i].contains);
}

int DynamicChordalGraph::allocate_node(std::vector<unsigned char> bits) {
    int id;
    if (!free_nodes_.empty()) {
        id = free_nodes_.back();
        free_nodes_.pop_back();
    } else {
        id = active_count_;
        while (id < static_cast<int>(nodes_.size()) && nodes_[id].active) ++id;
    }
    if (id >= static_cast<int>(nodes_.size()))
        throw std::logic_error("clique-tree node capacity exceeded");
    auto& node = nodes_[id];
    if (node.active || !node.incident.empty())
        throw std::logic_error("recycled clique-tree node is active");
    node.active = true;
    node.contains = std::move(bits);
    node.size = count_members(node.contains);
    ++active_count_;
    return id;
}

int DynamicChordalGraph::link(int a, int b, int weight) {
    if (a == b || !nodes_[a].active || !nodes_[b].active)
        throw std::logic_error("invalid clique-tree link");
    int id;
    if (!free_edges_.empty()) {
        id = free_edges_.back();
        free_edges_.pop_back();
    } else {
        id = static_cast<int>(edges_.size());
        edges_.emplace_back();
    }
    nodes_[a].incident.push_back(id);
    const auto it_a = std::prev(nodes_[a].incident.end());
    nodes_[b].incident.push_back(id);
    const auto it_b = std::prev(nodes_[b].incident.end());
    auto& edge = edges_[id];
    edge.active = true;
    edge.a = a; edge.b = b; edge.weight = weight;
    edge.in_a = it_a; edge.in_b = it_b;
    return id;
}

void DynamicChordalGraph::cut(int id) {
    auto& edge = edges_[id];
    if (!edge.active) throw std::logic_error("cutting inactive tree edge");
    nodes_[edge.a].incident.erase(edge.in_a);
    nodes_[edge.b].incident.erase(edge.in_b);
    edge.active = false;
    free_edges_.push_back(id);
}

int DynamicChordalGraph::overlap(int a, int b) const {
    int count = 0;
    for (int v = 0; v < graph_.vertex_count(); ++v)
        count += nodes_[a].contains[v] && nodes_[b].contains[v];
    return count;
}

int DynamicChordalGraph::clique_of_edge(int u, int v) const {
    int found = -1;
    for (int i = 0; i < static_cast<int>(nodes_.size()); ++i) {
        if (!nodes_[i].active || !nodes_[i].contains[u] ||
            !nodes_[i].contains[v]) continue;
        if (found != -1) return -1;
        found = i;
    }
    return found;
}

// Ibarra (2008), Theorem 4: an existing edge is deletable iff it
// belongs to exactly one maximal clique.
bool DynamicChordalGraph::delete_query(int u, int v) const {
    if (!graph_.has_edge(u, v)) return false;
    return fast_ ? fast_->count_common_cliques(u,v)==1 : clique_of_edge(u,v)>=0;
}

DynamicChordalGraph::EndpointPair
DynamicChordalGraph::closest_pair(int u, int v) const {
    // Section 3.3: path between the closest clique nodes containing
    // u and v respectively; clique membership subtrees are connected.
    EndpointPair pair;
    const int capacity = static_cast<int>(nodes_.size());
    int start = -1;
    for (int i = 0; i < capacity; ++i)
        if (nodes_[i].active && nodes_[i].contains[u]) {
            start = i; break;
        }
    if (start < 0) throw std::logic_error("vertex absent from clique tree");
    std::vector<int> previous(capacity, -1), via(capacity, -1);
    std::queue<int> q;
    q.push(start); previous[start] = start;
    int target = -1;
    while (!q.empty()) {
        const int current = q.front(); q.pop();
        if (nodes_[current].contains[v]) {
            target = current; break;
        }
        for (int id : nodes_[current].incident) {
            const auto& edge = edges_[id];
            const int next = edge.a == current ? edge.b : edge.a;
            if (previous[next] >= 0) continue;
            previous[next] = current; via[next] = id;
            q.push(next);
        }
    }
    if (target < 0) throw std::logic_error("clique tree disconnected");
    int x = target;
    while (x != start && !nodes_[x].contains[u]) x = previous[x];
    if (!nodes_[x].contains[u])
        throw std::logic_error("u-clique subtree path invalid");
    pair.x = x; pair.y = target;
    int current = target;
    while (current != x) {
        pair.path_edges.push_back(via[current]);
        current = previous[current];
    }
    return pair;
}

// Ibarra Theorems 5-6: chordality iff min edge weight on closest
// clique-subtree path equals the endpoint clique intersection size.
bool DynamicChordalGraph::insert_query(int u, int v) const {
    if (graph_.has_edge(u, v) || u == v) return false;
    const auto pair = closest_pair(u, v);
    if (pair.path_edges.empty()) return false;
    const int common = overlap(pair.x, pair.y);
    int minimum = graph_.vertex_count();
    for (int id : pair.path_edges)
        minimum = std::min(minimum, edges_[id].weight);
    return minimum == common;
}

void DynamicChordalGraph::contract(int keep, int drop) {
    if (keep == drop) throw std::logic_error("contracting a node into itself");
    std::vector<std::pair<int, int>> neighbors;
    for (int id : nodes_[drop].incident) {
        const auto& edge = edges_[id];
        const int other = edge.a == drop ? edge.b : edge.a;
        neighbors.emplace_back(other, id);
    }
    for (const auto& [other, id] : neighbors) {
        const int weight = edges_[id].weight;
        cut(id);
        if (other != keep) link(keep, other, weight);
    }
    nodes_[drop].active = false;
    nodes_[drop].contains.clear();
    nodes_[drop].size = 0;
    free_nodes_.push_back(drop);
    --active_count_;
}

// Ibarra Section 3.2 Delete: replace unique K_x by K_x-{v},
// K_x-{u}; redistribute incident edges, then contract if subsumed.
bool DynamicChordalGraph::delete_edge(int u, int v) {
    if (!delete_query(u, v)) return false;
    const int x = clique_of_edge(u, v);
    const int k = nodes_[x].size;
    if (fast_) fast_->remove_clique(x);
    std::vector<unsigned char> xu = nodes_[x].contains, xv = xu;
    xu[v] = 0; xv[u] = 0;
    struct Neighbor { int other, weight, side; };
    std::vector<Neighbor> neighbors;
    int absorb_u = -1, absorb_v = -1;
    for (int id : nodes_[x].incident) {
        const auto& edge = edges_[id];
        const int other = edge.a == x ? edge.b : edge.a;
        const int side = nodes_[other].contains[u] ? 0 :
                         (nodes_[other].contains[v] ? 1 : 2);
        neighbors.push_back({other, edge.weight, side});
        if (edge.weight == k - 1 && side == 0) absorb_u = other;
        if (edge.weight == k - 1 && side == 1) absorb_v = other;
    }
    const std::vector<int> old_edges(
        nodes_[x].incident.begin(), nodes_[x].incident.end());
    for (int id : old_edges) cut(id);
    nodes_[x].contains = std::move(xu);
    nodes_[x].size = k - 1;
    const int x2 = allocate_node(std::move(xv));
    for (const auto& neighbor : neighbors)
        link(neighbor.side == 1 ? x2 : x,
             neighbor.other, neighbor.weight);
    link(x, x2, k - 2);
    if (absorb_u >= 0) contract(absorb_u, x);
    if (absorb_v >= 0) contract(absorb_v, x2);
    graph_.remove_edge(u, v);
    if (fast_) {
        if (nodes_[x].active) fast_->add_clique(x,nodes_[x].contains);
        if (nodes_[x2].active) fast_->add_clique(x2,nodes_[x2].contains);
        fast_->rebalance(static_cast<int>(graph_.edge_count()));
    }
    return true;
}

// Ibarra Section 3.4 Insert: Theorem 6 tree-edge swap, insert
// K_z=(K_x intersect K_y) union {u,v}, and contract subsumed nodes.
bool DynamicChordalGraph::insert_edge(int u, int v) {
    if (!insert_query(u, v)) return false;
    const auto pair = closest_pair(u, v);
    const int x = pair.x, y = pair.y;
    const int common = overlap(x, y);
    if (pair.path_edges.size() != 1) {
        int minimum = pair.path_edges.front();
        for (int id : pair.path_edges)
            if (edges_[id].weight < edges_[minimum].weight) minimum = id;
        cut(minimum);
        link(x, y, common);
    }
    int xy = -1;
    for (int id : nodes_[x].incident) {
        const auto& edge = edges_[id];
        if ((edge.a == x && edge.b == y) ||
            (edge.a == y && edge.b == x)) {
            xy = id; break;
        }
    }
    if (xy < 0) throw std::logic_error("insertion tree edge missing");
    std::vector<unsigned char> bits(graph_.vertex_count(), 0);
    for (int i = 0; i < graph_.vertex_count(); ++i)
        bits[i] = nodes_[x].contains[i] && nodes_[y].contains[i];
    bits[u] = bits[v] = 1;
    const bool drop_x = nodes_[x].size == common + 1;
    const bool drop_y = nodes_[y].size == common + 1;
    if (fast_) {
        if (drop_x) fast_->remove_clique(x);
        if (drop_y) fast_->remove_clique(y);
    }
    cut(xy);
    const int z = allocate_node(std::move(bits));
    link(x, z, common + 1);
    link(z, y, common + 1);
    if (drop_x) contract(z, x);
    if (drop_y) contract(z, y);
    graph_.add_edge(u, v);
    if (fast_) {
        fast_->add_clique(z,nodes_[z].contains);
        fast_->rebalance(static_cast<int>(graph_.edge_count()));
    }
    return true;
}

CliqueTree DynamicChordalGraph::snapshot() const {
    CliqueTree tree;
    std::vector<int> index(nodes_.size(), -1);
    for (int id = 0; id < static_cast<int>(nodes_.size()); ++id) {
        if (!nodes_[id].active) continue;
        index[id] = static_cast<int>(tree.cliques.size());
        std::vector<int> clique;
        for (int v = 0; v < graph_.vertex_count(); ++v)
            if (nodes_[id].contains[v]) clique.push_back(v);
        tree.cliques.push_back(std::move(clique));
    }
    for (const auto& edge : edges_) {
        if (!edge.active) continue;
        tree.edges.push_back({index[edge.a], index[edge.b],
                              edge.weight, edge.weight == 0});
    }
    return tree;
}
} // namespace mtp
