#include "mtp/dynamic_ibarra_a2.hpp"
#include "mtp/clique_tree_blair_peyton.hpp"
#include "mtp/maximal_cliques.hpp"
#include "mtp/mcs.hpp"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {
using mtp::Graph;
using mtp::SleatorTarjanChordalGraphA2;
using mtp::DynamicChordalGraph;
void require(bool flag,const char* message) {
    if(!flag)throw std::runtime_error(message);
}
using Cliques=std::vector<std::vector<int>>;
Cliques canonical(Cliques cliques) {
    for(auto& k:cliques)std::sort(k.begin(),k.end());
    std::sort(cliques.begin(),cliques.end());
    return cliques;
}
int intersect(const std::vector<int>& a,const std::vector<int>& b) {
    int count=0;
    for(int u:a)
        count+=std::find(b.begin(),b.end(),u)!=b.end();
    return count;
}
void check_tree(const SleatorTarjanChordalGraphA2& dynamic) {
    const Graph& g=dynamic.graph();
    const auto order=mtp::maximum_cardinality_search(g);
    require(mtp::is_perfect_elimination_ordering(g,order),
            "dynamic graph lost chordality");
    const auto tree=dynamic.snapshot();
    const int k=static_cast<int>(tree.cliques.size());
    require(canonical(tree.cliques)==canonical(
        mtp::extract_maximal_cliques(g,order)),"wrong updated maximal cliques");
    require(static_cast<int>(tree.edges.size())==(k?k-1:0),
            "incorrect updated tree edge count");
    std::vector<std::vector<int>> adj(k);
    for(const auto& edge:tree.edges) {
        require(edge.first>=0&&edge.second>=0&&edge.first<k&&
                edge.second<k&&edge.first!=edge.second,"invalid edge endpoints");
        const int weight=intersect(tree.cliques[edge.first],
                                   tree.cliques[edge.second]);
        require(edge.weight==weight,"incorrect edge-intersection weight");
        require(edge.dummy==(weight==0),"dummy edge mismatch");
        adj[edge.first].push_back(edge.second);
        adj[edge.second].push_back(edge.first);
    }
    for(int v=0;v<g.vertex_count();++v) {
        int first=-1,expected=0;
        std::vector<bool> belongs(k,false),seen(k,false);
        for(int i=0;i<k;++i) {
            if(std::find(tree.cliques[i].begin(),tree.cliques[i].end(),v)
               !=tree.cliques[i].end()) {
                belongs[i]=true;++expected;first=i;
            }
        }
        require(expected>0,"vertex absent from cliques");
        std::queue<int> q;
        q.push(first);seen[first]=true;
        int count=0;
        while(!q.empty()) {
            const int x=q.front();q.pop();++count;
            for(int y:adj[x]) {
                if(belongs[y]&&!seen[y]) {
                    seen[y]=true;q.push(y);
                }
            }
        }
        require(count==expected,"induced-subtree property violated");
    }
}
void run_sequence(Graph graph,std::mt19937& random,int steps,bool fast_delete) {
    SleatorTarjanChordalGraphA2 a2(graph);
    DynamicChordalGraph baseline(std::move(graph));
    if(fast_delete)a2.enable_fast_delete_queries();
    const int n=a2.graph().vertex_count();
    check_tree(a2);
    a2.check_dynamic_tree_invariants();
    for(int step=0;step<steps;++step) {
        std::vector<std::pair<int,int>> permissible;
        for(int u=0;u<n;++u)for(int v=u+1;v<n;++v) {
            const bool insA=a2.insert_query(u,v);
            const bool insB=baseline.insert_query(u,v);
            const bool delA=a2.delete_query(u,v);
            const bool delB=baseline.delete_query(u,v);
            require(insA==insB,"a2 insert query disagrees with Ibarra oracle");
            require(delA==delB,"a2 delete query disagrees with Ibarra oracle");
            if(insB||delB)permissible.emplace_back(u,v);
        }
        if(permissible.empty())break;
        const auto [u,v]=permissible[random()%permissible.size()];
        bool a,b;
        if(a2.graph().has_edge(u,v)) {
            a=a2.delete_edge(u,v);
            b=baseline.delete_edge(u,v);
        } else {
            a=a2.insert_edge(u,v);
            b=baseline.insert_edge(u,v);
        }
        require(a&&b,"permitted a2 update rejected");
        check_tree(a2);
        a2.check_dynamic_tree_invariants();
    }
}
void exhaustive() {
    std::mt19937 rng(20261119);
#ifdef __SANITIZE_ADDRESS__
    constexpr int max_n=3;
#else
    constexpr int max_n=5;
#endif
    for(int n=0;n<=max_n;++n) {
        std::vector<std::pair<int,int>> pairs;
        for(int u=0;u<n;++u)for(int v=u+1;v<n;++v)
            pairs.emplace_back(u,v);
        const int count=1<<static_cast<int>(pairs.size());
        for(int mask=0;mask<count;++mask) {
            Graph g(n);
            for(int i=0;i<static_cast<int>(pairs.size());++i)
                if(mask&(1<<i))g.add_edge(pairs[i].first,pairs[i].second);
            if(!mtp::is_perfect_elimination_ordering(
                   g,mtp::maximum_cardinality_search(g)))continue;
            run_sequence(g,rng,8,false);
            run_sequence(std::move(g),rng,6,true);
        }
    }
}
void randomized() {
    std::mt19937 rng(20261119);
#ifdef __SANITIZE_ADDRESS__
    constexpr int trials=6,steps=5;
#else
    constexpr int trials=60,steps=20;
#endif
    for(int trial=0;trial<trials;++trial) {
        const int n=5+static_cast<int>(rng()%6);
        Graph g(0);
        for(int size=0;size<n;++size) {
            Graph next(size+1);
            for(int u=0;u<size;++u)
                for(int v:g.neighbors(u))if(u<v)next.add_edge(u,v);
            if(size>0) {
                const auto cliques=mtp::extract_maximal_cliques(
                    g,mtp::maximum_cardinality_search(g));
                const auto& k=cliques[rng()%cliques.size()];
                for(int v:k)if(rng()%3)next.add_edge(size,v);
            }
            g=std::move(next);
        }
        run_sequence(std::move(g),rng,steps,trial%2==0);
    }
}
// Repeat insertion/deletion through a clique tree with two high-degree
// adjacent cliques, using a published Blair–Peyton clique tree. This case
// is adversarial for Section 4 contraction and source-method A2 integration.
void adversarial_two_hub_updates() {
    std::mt19937 rng(20261119);
    for(const int k : {12,24}) {
        const int n=2*k+3;
        std::vector<int> labels(n);
        std::iota(labels.begin(),labels.end(),0);
        Graph chosen(n);
        int u=-1,v=-1;
        bool found=false;
        for(int attempt=0;attempt<256&&!found;++attempt) {
            if(attempt)std::shuffle(labels.begin(),labels.end(),rng);
            Graph g(n);
            g.add_edge(labels[0],labels[1]);
            g.add_edge(labels[0],labels[2]);
            for(int j=0;j<k;++j) {
                g.add_edge(labels[1],labels[3+j]);
                g.add_edge(labels[2],labels[3+k+j]);
            }
            const auto tree=mtp::build_clique_tree_blair_peyton(
                g,mtp::maximum_cardinality_search(g));
            std::vector<int> degree(tree.cliques.size());
            for(const auto& edge:tree.edges) {
                ++degree[edge.first];++degree[edge.second];
            }
            int a=-1,b=-1;
            for(int j=0;j<static_cast<int>(tree.cliques.size());++j) {
                auto clique=tree.cliques[j];
                if(clique.size()!=2 ||
                   std::find(clique.begin(),clique.end(),labels[0])==clique.end())
                    continue;
                if(std::find(clique.begin(),clique.end(),labels[1])!=clique.end())
                    a=degree[j];
                if(std::find(clique.begin(),clique.end(),labels[2])!=clique.end())
                    b=degree[j];
            }
            if(a>=k/2&&b>=k/2) {
                found=true;
                chosen=std::move(g);
                u=labels[1];v=labels[2];
            }
        }
        require(found,"failed to create two-high-degree clique case");
        SleatorTarjanChordalGraphA2 a2(chosen);
        DynamicChordalGraph baseline(chosen);
        if(k==24)a2.enable_fast_delete_queries();
        check_tree(a2);
        for(int rep=0;rep<7;++rep) {
            require(a2.insert_query(u,v)&&baseline.insert_query(u,v),
                    "adversarial insertion should be permitted");
            require(a2.insert_edge(u,v)&&baseline.insert_edge(u,v),
                    "adversarial insertion failed");
            check_tree(a2);
            a2.check_dynamic_tree_invariants();
            require(a2.delete_query(u,v)==baseline.delete_query(u,v),
                    "adversarial deletion query mismatch");
            require(a2.delete_edge(u,v)&&baseline.delete_edge(u,v),
                    "adversarial deletion failed");
            check_tree(a2);
            a2.check_dynamic_tree_invariants();
            for(int trial=0;trial<20;++trial) {
                const int p=int(rng()%n),q=int(rng()%n);
                if(p==q)continue;
                require(a2.insert_query(p,q)==baseline.insert_query(p,q),
                        "adversarial random insertion-query mismatch");
                require(a2.delete_query(p,q)==baseline.delete_query(p,q),
                        "adversarial random deletion-query mismatch");
            }
        }
    }
}
} // namespace
int main() {
    try {
        exhaustive();
        randomized();
        adversarial_two_hub_updates();
        std::cout << "PASS: source-faithful ST-backed A2 path queries versus independent A1 API\n"
                  << "PASS: exhaustive small chordal initial graphs, random updates\n"
                  << "PASS: clique tree invariants and fast deletion index\n"
                  << "PASS: repeated high-degree two-hub insertion/deletion cycles\n";
    } catch(const std::exception& e) {
        std::cerr << "FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
