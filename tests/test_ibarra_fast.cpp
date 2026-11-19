#include "mtp/dynamic_ibarra.hpp"
#include "mtp/maximal_cliques.hpp"
#include "mtp/mcs.hpp"

#include <algorithm>
#include <iostream>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace mtp;
using Cliques = std::vector<std::vector<int>>;

static void require(bool ok, const char* text) {
    if (!ok) throw std::runtime_error(text);
}
static Cliques canonical(Cliques value) {
    for (auto& clique : value) std::sort(clique.begin(), clique.end());
    std::sort(value.begin(), value.end());
    return value;
}
static int weight(const std::vector<int>& a, const std::vector<int>& b) {
    int answer = 0;
    for (int u : a)
        if (std::find(b.begin(), b.end(), u) != b.end()) ++answer;
    return answer;
}
static void validate(const DynamicChordalGraph& dynamic) {
    const auto& graph = dynamic.graph();
    const auto peo = maximum_cardinality_search(graph);
    require(is_perfect_elimination_ordering(graph, peo),
            "updated graph is not chordal");
    const auto tree = dynamic.snapshot();
    const int k = static_cast<int>(tree.cliques.size());
    require(canonical(tree.cliques) ==
            canonical(extract_maximal_cliques(graph, peo)),
            "incorrect updated maximal cliques");
    require(static_cast<int>(tree.edges.size()) == std::max(0, k-1),
            "incorrect updated tree edge count");
    std::vector<std::vector<int>> adjacency(k);
    for (const auto& e : tree.edges) {
        require(e.first >= 0 && e.second >= 0 &&
                e.first < k && e.second < k && e.first != e.second,
                "invalid tree edge");
        require(e.weight == weight(tree.cliques[e.first], tree.cliques[e.second]),
                "incorrect separator weight");
        require(e.dummy == (e.weight == 0), "incorrect dummy edge");
        adjacency[e.first].push_back(e.second);
        adjacency[e.second].push_back(e.first);
    }
    for (int vertex = 0; vertex < graph.vertex_count(); ++vertex) {
        int first = -1, count = 0;
        std::vector<char> member(k), visited(k);
        for (int i = 0; i < k; ++i) {
            if (std::find(tree.cliques[i].begin(), tree.cliques[i].end(),
                          vertex) == tree.cliques[i].end()) continue;
            member[i] = true; count++; first = i;
        }
        require(count > 0, "vertex absent from all maximal cliques");
        std::queue<int> q;
        q.push(first); visited[first] = 1;
        int reached = 0;
        while (!q.empty()) {
            int i = q.front(); q.pop(); ++reached;
            for (int j : adjacency[i]) {
                if (!member[j] || visited[j]) continue;
                visited[j] = 1; q.push(j);
            }
        }
        require(reached == count, "running intersection property broken");
    }
}
static void run_sequence(Graph graph, std::mt19937& rng, int steps) {
    DynamicChordalGraph dynamic(std::move(graph));
    dynamic.enable_fast_delete_queries();
    validate(dynamic);
    const int n = dynamic.graph().vertex_count();
    for (int step = 0; step < steps; ++step) {
        std::vector<std::pair<int,int>> allowed;
        for (int u = 0; u < n; ++u)
            for (int v = u+1; v < n; ++v) {
                Graph changed = dynamic.graph();
                const bool existing = changed.has_edge(u, v);
                if (existing) changed.remove_edge(u,v);
                else changed.add_edge(u,v);
                const bool oracle = is_perfect_elimination_ordering(
                    changed, maximum_cardinality_search(changed));
                const bool actual = existing ?
                    dynamic.delete_query(u,v) : dynamic.insert_query(u,v);
                require(oracle == actual, "update query differs from oracle");
                if (oracle) allowed.push_back({u,v});
            }
        if (allowed.empty()) break;
        const auto [u,v] = allowed[rng() % allowed.size()];
        const bool existing = dynamic.graph().has_edge(u,v);
        require(existing ? dynamic.delete_edge(u,v) :
                           dynamic.insert_edge(u,v),
                "permitted update unexpectedly rejected");
        validate(dynamic);
    }
}
int main() {
    try {
        std::mt19937 rng(20261119);
        // All chordal graphs up to 5 vertices; independent query oracle
        // for every edge/nonedge, followed by four random legal updates.
        for (int n=0; n<=5; ++n) {
            std::vector<std::pair<int,int>> edges;
            for (int u=0;u<n;++u)
                for (int v=u+1;v<n;++v) edges.push_back({u,v});
            for (int mask=0;mask<(1<<static_cast<int>(edges.size()));++mask) {
                Graph graph(n);
                for (int p=0;p<static_cast<int>(edges.size());++p)
                    if (mask&(1<<p))
                        graph.add_edge(edges[p].first,edges[p].second);
                if (is_perfect_elimination_ordering(
                        graph,maximum_cardinality_search(graph)))
                    run_sequence(std::move(graph),rng,4);
            }
        }
        // Longer random alternating deletion/insertion sequences.
        for (int trial=0;trial<200;++trial) {
            const int n=6+static_cast<int>(rng()%7);
            Graph graph(0);
            for (int size=0;size<n;++size) {
                Graph next(size+1);
                for(int u=0;u<size;++u)
                    for(int v:graph.neighbors(u))
                        if(u<v) next.add_edge(u,v);
                if(size) {
                    auto cliques=extract_maximal_cliques(
                        graph,maximum_cardinality_search(graph));
                    const auto& clique=cliques[rng()%cliques.size()];
                    for(int v:clique)if(rng()%3)next.add_edge(size,v);
                }
                graph=std::move(next);
            }
            run_sequence(std::move(graph),rng,25);
        }
        std::cout << "PASS: Ibarra Sections 3-4 plus Section 6 fast deletion-query index\n"
                  << "PASS: oracle comparison and clique-tree invariants\n";
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
