#include "mtp/sparse_ibarra_a1.hpp"
#include "mtp/dynamic_ibarra.hpp"
#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace mtp;
static void require(bool ok,const char* why) {
    if(!ok)throw std::runtime_error(why);
}
static void verify_sparse(const SparseIbarraChordalGraphA1& d) {
    const int n=d.graph().vertex_count();
    std::size_t total=0;
    for(const auto& k:d.snapshot().cliques)total+=k.size();
    require(total==d.stored_memberships(),"incidence count differs from snapshot");
    require(total<=static_cast<std::size_t>(n)+d.graph().edge_count(),
            "chordal clique incidence exceeds n+m");
}

static void compare_queries(SparseIbarraChordalGraphA1& sparse,DynamicChordalGraph& dense) {
    const int n=sparse.graph().vertex_count();
    require(sparse.graph().edge_count()==dense.graph().edge_count(),
            "graph edge count differs");
    for(int u=0;u<n;++u)for(int v=u+1;v<n;++v) {
        require(sparse.insert_query(u,v)==dense.insert_query(u,v),
                "sparse/dense insertion-query mismatch");
        require(sparse.delete_query(u,v)==dense.delete_query(u,v),
                "sparse/dense deletion-query mismatch");
    }
    verify_sparse(sparse);
}
static void run() {
    // Adversarial star: Theta(n) maximal cliques, only O(n) stored memberships.
    for(int n:{64,128,256}) {
        Graph g(n);
        for(int v=1;v<n;++v)g.add_edge(0,v);
        SparseIbarraChordalGraphA1 sparse(g);
        DynamicChordalGraph dense(g);
        verify_sparse(sparse);
        require(sparse.stored_memberships()==static_cast<std::size_t>(2*(n-1)),
                "star has wrong incidence size");
        compare_queries(sparse,dense);
        for(int t=0;t<24;++t) {
            const int a=1+t%(n-2),b=a+1;
            require(sparse.insert_edge(a,b)==dense.insert_edge(a,b),
                    "star insert mismatch");
            require(sparse.delete_edge(a,b)==dense.delete_edge(a,b),
                    "star delete mismatch");
            verify_sparse(sparse);
        }
        compare_queries(sparse,dense);
    }
    // Empty graph uses exactly n incidence records, not n^2.
    for(int n:{0,1,100,1000,3000}) {
        SparseIbarraChordalGraphA1 sparse{Graph(n)};
        require(sparse.stored_memberships()==static_cast<std::size_t>(n),
                "empty graph non-linear clique incidence");
        require(sparse.maximal_clique_count()==n,"empty graph clique count");
        verify_sparse(sparse);
    }
    // Large clique, repeated permissible deletion/insert with node recycling.
    for(int n:{30,80}) {
        Graph g(n);
        for(int u=0;u<n;++u)for(int v=u+1;v<n;++v)g.add_edge(u,v);
        SparseIbarraChordalGraphA1 sparse(g);
        DynamicChordalGraph dense(g);
        for(int t=0;t<35;++t) {
            const int u=t%n,v=(t+1)%n;
            require(sparse.delete_edge(u,v)==dense.delete_edge(u,v),
                    "dense delete mismatch");
            require(sparse.insert_edge(u,v)==dense.insert_edge(u,v),
                    "dense insert mismatch");
            verify_sparse(sparse);
        }
        compare_queries(sparse,dense);
    }

    // Bulk input with arbitrary ordering has deterministic linear-time
    // bucket-sort construction. Verify adjacency and duplicate rejection.
    std::mt19937 rng(20261121);
    for(int trial=0;trial<50;++trial) {
        const int n=5+static_cast<int>(rng()%80);
        std::vector<std::pair<int,int>> es;
        Graph expected(n);
        for(int u=0;u<n;++u)for(int v=u+1;v<n;++v)
            if(rng()%9==0) {
                es.emplace_back(u,v);
                expected.add_edge(u,v);
            }
        std::shuffle(es.begin(),es.end(),rng);
        Graph actual=Graph::from_edges(n,es);
        require(actual.edge_count()==expected.edge_count(),
                "bulk graph edge count mismatch");
        for(int v=0;v<n;++v)
            for(int u=0;u<n;++u)
                require(actual.has_edge(u,v)==expected.has_edge(u,v),
                        "bulk graph membership mismatch");
        if(!es.empty()) {
            es.push_back(es.front());
            bool rejected=false;
            try{(void)Graph::from_edges(n,es);}
            catch(const std::invalid_argument&){rejected=true;}
            require(rejected,"duplicate bulk input accepted");
        }
    }
}

int main() {
    try{run();std::cout<<"PASS: sparse A1 incidence bounds, adversarial "
                         "updates and linear bulk graph loader\n";}
    catch(const std::exception& e) {
        std::cerr<<"FAIL: "<<e.what()<<"\n";
        return 1;
    }
}
