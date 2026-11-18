#include "mtp/clique_graph.hpp"
#include "mtp/clique_tree.hpp"
#include "mtp/graph.hpp"
#include "mtp/maximal_cliques.hpp"
#include "mtp/mcs.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

// Exhaustive Prüfer-sequence spanning-tree oracle; independent of Kruskal.
// For small k, it checks the maximum possible sum of pairwise clique
// intersection sizes over ALL labeled trees on k nodes.
int brute_best_weight(const mtp::WeightedCliqueGraph& graph) {
    const int k = static_cast<int>(graph.cliques.size());
    require(k <= 6, "Pruefer oracle limited to six clique nodes");
    if (k <= 1) return 0;
    std::vector<std::vector<int>> w(k, std::vector<int>(k, 0));
    for (int i = 0; i < k; ++i)
        for (int j = i + 1; j < k; ++j)
            w[i][j] = w[j][i] = mtp::clique_intersection_size(
                graph.cliques[i], graph.cliques[j]);
    int max_weight = std::numeric_limits<int>::min();
    std::vector<int> sequence(static_cast<std::size_t>(k - 2), 0);
    auto inspect = [&]() {
        std::vector<int> degree(k, 1);
        for (int p : sequence) ++degree[p];
        int sum = 0;
        for (int p : sequence) {
            int leaf = -1;
            for (int v = 0; v < k; ++v) if (degree[v] == 1) { leaf = v; break; }
            require(leaf >= 0, "Pruefer leaf not found");
            sum += w[leaf][p];
            --degree[leaf];
            --degree[p];
        }
        int a = -1, b = -1;
        for (int v = 0; v < k; ++v) {
            if (degree[v] == 1) { if (a == -1) a = v; else b = v; }
        }
        require(a >= 0 && b >= 0, "Pruefer final pair invalid");
        max_weight = std::max(max_weight, sum + w[a][b]);
    };
    auto generate = [&](auto&& self, int position) -> void {
        if (position == k - 2) { inspect(); return; }
        for (int x = 0; x < k; ++x) {
            sequence[position] = x;
            self(self, position+1);
        }
    };
    generate(generate, 0);
    return max_weight;
}

void check_chordal(const mtp::Graph& graph, bool do_brute) {
    const auto peo = mtp::maximum_cardinality_search(graph);
    if (!mtp::is_perfect_elimination_ordering(graph, peo)) return;
    const auto maximal_cliques = mtp::extract_maximal_cliques(graph, peo);
    const auto wg = mtp::build_weighted_clique_graph(maximal_cliques);
    const auto tree = mtp::build_clique_tree(wg);
    require(mtp::verify_clique_tree(wg, tree), "clique tree verification failed");
    int weight = 0;
    for (const auto& edge : tree.edges) {
        weight += edge.weight;
        require((edge.weight == 0) == edge.dummy, "bad dummy edge classification");
    }
    if (do_brute && wg.cliques.size() <= 6) {
        require(weight == brute_best_weight(wg), "clique tree not max-weight");
    }
    // Check the original W_G edge set and edge weights independently.
    int expected_edges = 0;
    for (int i = 0; i < static_cast<int>(wg.cliques.size()); ++i) {
        for (int j = i + 1; j < static_cast<int>(wg.cliques.size()); ++j) {
            int intersection = 0;
            for (int u : wg.cliques[i]) {
                if (std::find(wg.cliques[j].begin(), wg.cliques[j].end(), u) !=
                    wg.cliques[j].end()) ++intersection;
            }
            if (intersection) ++expected_edges;
            const int count = static_cast<int>(std::count_if(wg.edges.begin(),wg.edges.end(),
                [&](const mtp::CliqueEdge& e) { return e.first==i && e.second==j && e.weight==intersection; }));
            require(count == (intersection > 0 ? 1 : 0), "clique graph missing/wrong intersection edge");
        }
    }
    require(static_cast<int>(wg.edges.size()) == expected_edges, "extra clique graph edge");
}

void targeted_tests() {
    mtp::Graph sample(7);
    for (const auto& [u,v] : std::vector<std::pair<int,int>>{
             {0,1},{1,3},{0,2},{2,3},{2,4},
             {2,5},{4,5},{3,6},{0,3},{1,2}}) sample.add_edge(u,v);
    const auto peo = mtp::maximum_cardinality_search(sample);
    const auto cliques = mtp::extract_maximal_cliques(sample, peo);
    const auto cg = mtp::build_weighted_clique_graph(cliques);
    require(cg.cliques.size() == 3, "sample should have 3 maximal cliques");
    require(cg.edges.size() == 2, "sample clique graph must have 2 edges");
    for (const auto& edge : cg.edges) require(edge.weight == 1, "sample intersection size must be 1");
    check_chordal(sample, true);

    mtp::Graph disconnected(4);
    disconnected.add_edge(0,1);
    disconnected.add_edge(2,3);
    const auto cg2=mtp::build_weighted_clique_graph(
        mtp::extract_maximal_cliques(disconnected,mtp::maximum_cardinality_search(disconnected)));
    require(cg2.edges.empty(), "disconnected two-clique W_G should have no positive edges");
    const auto tree=mtp::build_clique_tree(cg2);
    require(tree.edges.size()==1 && tree.edges[0].dummy && tree.edges[0].weight==0,
            "disconnected clique tree needs one zero weight dummy edge");
    require(mtp::verify_clique_tree(cg2,tree), "disconnected running intersection failed");
    auto corrupt=tree;
    corrupt.edges[0].dummy=false;
    require(!mtp::verify_clique_tree(cg2,corrupt), "bad dummy flag accepted");

    check_chordal(mtp::Graph(0), true);
    check_chordal(mtp::Graph(1), true);
    check_chordal(mtp::Graph(5), true);

    bool error=false;
    try { (void)mtp::build_weighted_clique_graph({{0,0}}); }
    catch(const std::invalid_argument&) { error=true; }
    require(error,"duplicate vertex in supplied clique not rejected");
}

void exhaustive_small_graphs() {
    int chordal=0;
    for (int n=1;n<=6;++n) {
        std::vector<std::pair<int,int>> pairs;
        for(int u=0;u<n;++u) for(int v=u+1;v<n;++v) pairs.emplace_back(u,v);
        for(int mask=0;mask<(1<<static_cast<int>(pairs.size()));++mask) {
            mtp::Graph graph(n);
            for(int j=0;j<static_cast<int>(pairs.size());++j)
                if((mask>>j)&1) graph.add_edge(pairs[j].first,pairs[j].second);
            const auto peo=mtp::maximum_cardinality_search(graph);
            if(!mtp::is_perfect_elimination_ordering(graph,peo)) continue;
            ++chordal;
            check_chordal(graph,n<=5); // independent brute MWST on n<=5
        }
    }
    require(chordal==19048, "unexpected exhaustive chordal count");
}

void random_graphs() {
    std::mt19937 rng(20261118);
    for(int trial=0;trial<600;++trial) {
        const int n=7+static_cast<int>(rng()%6);
        mtp::Graph current(0);
        for(int size=0;size<n;++size) {
            mtp::Graph next(size+1);
            for(int u=0;u<size;++u)
                for(int v:current.neighbors(u))
                    if(u<v) next.add_edge(u,v);
            if(size>0){
                auto peo=mtp::maximum_cardinality_search(current);
                auto candidates=mtp::extract_maximal_cliques(current,peo);
                const auto& clique=candidates[rng()%candidates.size()];
                for(int v:clique) if(rng()%3!=0) next.add_edge(size,v);
            }
            current=std::move(next);
        }
        check_chordal(current,false);
    }
}

} // namespace

int main() {
    try {
        targeted_tests();
        exhaustive_small_graphs();
        random_graphs();
        std::cout << "PASS: clique graph + clique tree targeted tests\n"
                  << "PASS: 19,048 chordal graphs on 1..6 vertices (running intersection)\n"
                  << "PASS: independent brute-force max spanning tree on n<=5\n"
                  << "PASS: 600 additional random chordal graphs\n";
    }catch(const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
