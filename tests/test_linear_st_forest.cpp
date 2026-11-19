#include "mtp/worst_case_dynamic_tree.hpp"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <stdexcept>
#include <vector>
using Forest=mtp::SleatorTarjanWorstCaseForest;
using Edge=Forest::Edge;
void need(bool p,const char* message) {
    if(!p)throw std::runtime_error(message);
}
std::vector<int> brute_path(int n,const std::vector<Edge>& edges,int u,int v){
    std::vector<std::vector<int>> adj(n);
    for(const auto& e:edges) {
        adj[e.u].push_back(e.v);
        adj[e.v].push_back(e.u);
    }
    std::vector<int> prev(n,-1);
    std::queue<int> q;q.push(u);prev[u]=u;
    while(!q.empty()){
        const int x=q.front();q.pop();
        for(const int y:adj[x])if(prev[y]==-1){
            prev[y]=x;q.push(y);
        }
    }
    if(prev[v]<0)return {};
    std::vector<int> out;
    for(int x=v;x!=u;x=prev[x])out.push_back(x);
    out.push_back(u);
    std::reverse(out.begin(),out.end());
    return out;
}
int weight(const std::vector<Edge>& edges,int a,int b){
    for(const auto& e:edges)if((e.u==a&&e.v==b)||(e.u==b&&e.v==a))
        return e.cost;
    throw std::runtime_error("missing oracle edge");
}
void test_one(int n,std::vector<Edge> edges,std::mt19937& rng){
    auto f=Forest::build_linear_forest(n,edges);
    const auto budget=f.static_build_steps();
    if(budget>100LL*n)
        throw std::runtime_error("static construction not linear, n="+
            std::to_string(n)+" steps="+std::to_string(budget));
    f.check_invariants();
    for(int rep=0;rep<150&&n>0;++rep){
        const int a=int(rng()%n),b=int(rng()%n);
        auto p=brute_path(n,edges,a,b);
        if(p.empty()){
            need(f.root(a)!=f.root(b),"connectivity mismatch after static build");
            continue;
        }
        need(f.path_length(a,b)==int(p.size()),"static path length wrong");
        int best=1000000000;
        for(int i=0;i<int(p.size());++i) {
            need(f.kth_on_path(a,b,i)==p[i],"static kth mismatch");
            if(i)best=std::min(best,weight(edges,p[i-1],p[i]));
        }
        need(f.min_on_path(a,b)==best,"static minimum edge mismatch");
    }
    f.check_invariants();
    if(!edges.empty()){
        const Edge e=edges[edges.size()/2];
        f.cut(e.u,e.v);
        edges.erase(std::find_if(edges.begin(),edges.end(),
            [&](const Edge& a) {
                return (a.u==e.u&&a.v==e.v)||(a.u==e.v&&a.v==e.u);
            }));
        need(f.root(e.u)!=f.root(e.v),"cut failed after static build");
        f.link(e.u,e.v,e.cost);
        edges.push_back(e);
        need(f.root(e.u)==f.root(e.v),"link failed after static build");
        need(f.min_on_path(e.u,e.v)==e.cost,"relinked edge weight wrong");
        f.evert(e.u);
        f.check_invariants();
    }
}
int main(){
    try {
        std::mt19937 rng(19831985);
        for(int n : {0,1,2,3,4,5,10,30,75,150,300,512}) {
            std::vector<Edge> path,star,double_star,random_tree;
            for(int v=1;v<n;++v) {
                path.push_back({v-1,v,int(rng()%201)-100});
                star.push_back({0,v,int(rng()%201)-100});
                double_star.push_back({v<=2?0:(v%2==0?1:2),v,
                                      int(rng()%201)-100});
                random_tree.push_back({int(rng()%v),v,int(rng()%201)-100});
            }
            test_one(n,path,rng);
            test_one(n,star,rng);
            test_one(n,random_tree,rng);
            if(n>2)test_one(n,double_star,rng);
        }
        std::cout<<"PASS: ST1983/BST1985 linear forest construction, "
                    "star, path, random, double-star, followed by "
                    "link/cut/evert and BFS path/min oracles\n";
    }catch(const std::exception& e) {
        std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;
    }
}
