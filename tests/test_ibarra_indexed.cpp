#include "mtp/indexed_dynamic_ibarra.hpp"
#include "mtp/maximal_cliques.hpp"
#include "mtp/mcs.hpp"
#include <algorithm>
#include <iostream>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {
using mtp::Graph;
using mtp::IndexedDynamicChordalGraph;
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
void check_tree(const IndexedDynamicChordalGraph& dynamic) {
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
    IndexedDynamicChordalGraph indexed(graph);
    DynamicChordalGraph baseline(std::move(graph));
    if(fast_delete)indexed.enable_fast_delete_queries();
    const int n=indexed.graph().vertex_count();
    check_tree(indexed);
    for(int step=0;step<steps;++step) {
        std::vector<std::pair<int,int>> permissible;
        for(int u=0;u<n;++u)for(int v=u+1;v<n;++v) {
            const bool insA=indexed.insert_query(u,v);
            const bool insB=baseline.insert_query(u,v);
            const bool delA=indexed.delete_query(u,v);
            const bool delB=baseline.delete_query(u,v);
            require(insA==insB,"indexed insert query disagrees with Ibarra oracle");
            require(delA==delB,"indexed delete query disagrees with Ibarra oracle");
            if(insB||delB)permissible.emplace_back(u,v);
        }
        if(permissible.empty())break;
        const auto [u,v]=permissible[random()%permissible.size()];
        bool a,b;
        if(indexed.graph().has_edge(u,v)) {
            a=indexed.delete_edge(u,v);
            b=baseline.delete_edge(u,v);
        } else {
            a=indexed.insert_edge(u,v);
            b=baseline.insert_edge(u,v);
        }
        require(a&&b,"permitted indexed update rejected");
        check_tree(indexed);
    }
}
void exhaustive() {
    std::mt19937 rng(20261119);
    for(int n=0;n<=5;++n) {
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
    for(int trial=0;trial<240;++trial) {
        const int n=7+static_cast<int>(rng()%7);
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
        run_sequence(std::move(g),rng,20,trial%2==0);
    }
}
} // namespace
int main() {
    try {
        exhaustive();
        randomized();
        std::cout << "PASS: indexed path queries versus independent A1 dynamic API\n"
                  << "PASS: exhaustive small chordal initial graphs, random updates\n"
                  << "PASS: clique tree invariants and fast deletion index\n";
    } catch(const std::exception& e) {
        std::cerr << "FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
