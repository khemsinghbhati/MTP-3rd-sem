// Deterministic search for two neighboring high-degree maximal cliques.
// The graph is a tree: central vertices r-u-v (two edges r-u,r-v),
// each central endpoint u,v has k pendant neighbors.
// Inserting u-v is chordality preserving and subsumes the r-u and r-v cliques.
// This DOES NOT prove a lower bound on running time: it measures the
// implementation's unbatched, ST link/cut operation counts.
#include "mtp/graph.hpp"
#include "mtp/mcs.hpp"
#include "mtp/clique_tree_blair_peyton.hpp"
#include "mtp/dynamic_ibarra_a2.hpp"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

struct Candidate {
    mtp::Graph graph;
    int u=-1,v=-1, degree_u=-1,degree_v=-1;
    explicit Candidate(int n):graph(n){}
};
Candidate make_candidate(int k,const std::vector<int>& labels) {
    const int n=2*k+3;
    Candidate cand(n);
    cand.u=labels[1];cand.v=labels[2];
    cand.graph.add_edge(labels[0],labels[1]);
    cand.graph.add_edge(labels[0],labels[2]);
    for(int i=0;i<k;++i) {
        cand.graph.add_edge(labels[1],labels[3+i]);
        cand.graph.add_edge(labels[2],labels[3+k+i]);
    }
    const auto peo=mtp::maximum_cardinality_search(cand.graph);
    const auto tree=mtp::build_clique_tree_blair_peyton(cand.graph,peo);
    std::vector<int> degree(tree.cliques.size(),0);
    for(const auto& edge:tree.edges) {
        ++degree[edge.first];
        ++degree[edge.second];
    }
    for(int id=0;id<static_cast<int>(tree.cliques.size());++id) {
        const auto& clique=tree.cliques[id];
        if(clique.size()!=2)continue;
        if(std::find(clique.begin(),clique.end(),labels[0])==clique.end())continue;
        if(std::find(clique.begin(),clique.end(),labels[1])!=clique.end())
            cand.degree_u=degree[id];
        if(std::find(clique.begin(),clique.end(),labels[2])!=clique.end())
            cand.degree_v=degree[id];
    }
    if(cand.degree_u<0||cand.degree_v<0)
        throw std::runtime_error("missing central clique");
    return cand;
}
int main() {
    std::mt19937 rng(20261119);
    std::cout<<"n,best_min_center_degree,degree_u,degree_v,st_links,st_cuts,st_calls\n";
    for(int k : {12,24,48}) {
        const int n=2*k+3;
        std::vector<int> order(n);
        std::iota(order.begin(),order.end(),0);
        Candidate best(n);
        int best_min=-1;
        for(int trial=0;trial<256;++trial) {
            if(trial>0)std::shuffle(order.begin(),order.end(),rng);
            Candidate cand=make_candidate(k,order);
            const int minimum=std::min(cand.degree_u,cand.degree_v);
            if(minimum>best_min) {
                best_min=minimum;
                best=std::move(cand);
            }
        }
        mtp::SleatorTarjanChordalGraphA2 a2(best.graph);
        if(!a2.insert_query(best.u,best.v) ||
           !a2.insert_edge(best.u,best.v))
            throw std::runtime_error("permitted central-edge insert failed");
        a2.check_dynamic_tree_invariants();
        const int links=a2.dynamic_tree_link_count_last_update();
        const int cuts=a2.dynamic_tree_cut_count_last_update();
        std::cout<<n<<','<<best_min<<','<<best.degree_u<<','
                 <<best.degree_v<<','<<links<<','<<cuts<<','
                 <<links+cuts<<'\n';
    }
}
