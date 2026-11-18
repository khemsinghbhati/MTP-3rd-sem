#include "mtp/mcs.hpp"

#include "mtp/graph.hpp"

#include <list>
#include <vector>

namespace mtp {

std::vector<int> maximum_cardinality_search(const Graph& graph) {
    const int n = graph.vertex_count();
    std::vector<int> ordering(static_cast<std::size_t>(n));
    if (n == 0) return ordering;

    std::vector<int> weight(static_cast<std::size_t>(n), 0);
    std::vector<bool> numbered(static_cast<std::size_t>(n), false);
    std::vector<std::list<int>> buckets(static_cast<std::size_t>(n));
    std::vector<std::list<int>::iterator> locations(static_cast<std::size_t>(n));

    // Use push_front to preserve deterministic bucket tie-breaking.
    // deterministic tie-breaking behavior for the supplied sample graph.
    for (int v = 0; v < n; ++v) {
        buckets[0].push_front(v);
        locations[static_cast<std::size_t>(v)] = buckets[0].begin();
    }

    int max_weight = 0;
    for (int position = n - 1; position >= 0; --position) {
        while (max_weight > 0 && buckets[static_cast<std::size_t>(max_weight)].empty()) {
            --max_weight;
        }
        auto& bucket = buckets[static_cast<std::size_t>(max_weight)];
        // Invariant: at least one unnumbered vertex remains, in some bucket.
        const int chosen = bucket.front();
        bucket.pop_front();
        ordering[static_cast<std::size_t>(position)] = chosen;
        numbered[static_cast<std::size_t>(chosen)] = true;

        for (const int neighbor : graph.neighbors(chosen)) {
            if (numbered[static_cast<std::size_t>(neighbor)]) continue;
            const int old_weight = weight[static_cast<std::size_t>(neighbor)];
            buckets[static_cast<std::size_t>(old_weight)].erase(
                locations[static_cast<std::size_t>(neighbor)]);
            const int new_weight = ++weight[static_cast<std::size_t>(neighbor)];
            buckets[static_cast<std::size_t>(new_weight)].push_front(neighbor);
            locations[static_cast<std::size_t>(neighbor)] =
                buckets[static_cast<std::size_t>(new_weight)].begin();
            if (new_weight > max_weight) max_weight = new_weight;
        }
    }
    return ordering;
}

bool is_perfect_elimination_ordering(const Graph& graph,
                                     const std::vector<int>& ordering) {
    const int n = graph.vertex_count();
    if (ordering.size() != static_cast<std::size_t>(n)) return false;

    std::vector<int> position(static_cast<std::size_t>(n), -1);
    for (int i = 0; i < n; ++i) {
        const int v = ordering[static_cast<std::size_t>(i)];
        if (v < 0 || v >= n || position[static_cast<std::size_t>(v)] != -1) {
            return false;
        }
        position[static_cast<std::size_t>(v)] = i;
    }

    // A complete PEO verification: for each u, all later neighbors other
    // than the earliest later neighbor p must also be adjacent to p.
    for (int u = 0; u < n; ++u) {
        int parent = -1;
        for (const int v : graph.neighbors(u)) {
            if (position[static_cast<std::size_t>(v)] > position[static_cast<std::size_t>(u)] &&
                (parent == -1 || position[static_cast<std::size_t>(v)] <
                                     position[static_cast<std::size_t>(parent)])) {
                parent = v;
            }
        }
        if (parent == -1) continue;
        for (const int v : graph.neighbors(u)) {
            if (v != parent &&
                position[static_cast<std::size_t>(v)] > position[static_cast<std::size_t>(u)] &&
                !graph.has_edge(parent, v)) {
                return false;
            }
        }
    }
    return true;
}

} // namespace mtp
