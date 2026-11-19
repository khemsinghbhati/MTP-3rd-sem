// Complexity audit for Ibarra (2008), Section 5 A2 Insert.
// This is a structural operation-count experiment, not a correctness proof.
// In a star, adding an edge between leaves is chordality-preserving. The
// existing A1 clique-tree contraction can redirect every incident edge of
// a high-degree clique, producing multiple ST link/cut calls.
#include "mtp/dynamic_ibarra_a2.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <algorithm>

int main() {
    std::cout<<"n,maximum_clique_tree_degree,st_links,st_cuts,st_total_calls\n";
    for(int n : {12,24,48,96,192,384}) {
        mtp::Graph g(n);
        for(int v=1;v<n;++v)g.add_edge(0,v);
        mtp::SleatorTarjanChordalGraphA2 dynamic(std::move(g));
        const auto tree=dynamic.snapshot();
        const int k=static_cast<int>(tree.cliques.size());
        std::vector<int> degree(k,0);
        for(const auto& edge:tree.edges) {
            ++degree[edge.first];++degree[edge.second];
        }
        int hub=0;
        for(int i=1;i<k;++i)
            if(degree[i]>degree[hub])hub=i;
        int u=-1;
        for(int v:tree.cliques[hub])if(v!=0)u=v;
        if(u<1)throw std::runtime_error("hub clique lacks a leaf");
        int v=1;
        if(v==u)v=2;
        if(!dynamic.insert_query(u,v) || !dynamic.insert_edge(u,v))
            throw std::runtime_error("star leaf-to-leaf insertion rejected");
        dynamic.check_dynamic_tree_invariants();
        std::cout<<n<<','<<degree[hub]<<','
                 <<dynamic.dynamic_tree_link_count_last_update()<<','
                 <<dynamic.dynamic_tree_cut_count_last_update()<<','
                 <<dynamic.dynamic_tree_link_count_last_update()+
                   dynamic.dynamic_tree_cut_count_last_update()<<'\n';
    }
}
