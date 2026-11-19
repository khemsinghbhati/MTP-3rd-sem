#include "mtp/worst_case_dynamic_tree.hpp"
#include <algorithm>
#include <iostream>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using Forest=mtp::SleatorTarjanWorstCaseForest;
static std::string stage;
struct Oracle {
    std::vector<std::vector<std::pair<int,int>>> adjacent;
    explicit Oracle(int n):adjacent(n){}
    [[nodiscard]] std::vector<int> path(int u,int v)const {
        const int n=static_cast<int>(adjacent.size());
        std::vector<int> parent(n,-1);
        std::queue<int> q;
        q.push(u);parent[u]=u;
        while(!q.empty()) {
            int x=q.front();q.pop();
            if(x==v)break;
            for(auto [y,c]:adjacent[x]) {
                (void)c;
                if(parent[y]<0){parent[y]=x;q.push(y);}
            }
        }
        if(parent[v]<0)return {};
        std::vector<int> result;
        for(int x=v;x!=u;x=parent[x])result.push_back(x);
        result.push_back(u);
        std::reverse(result.begin(),result.end());
        return result;
    }
    [[nodiscard]] int edge(int a,int b)const {
        for(auto [v,w]:adjacent[a])if(v==b)return w;
        throw std::runtime_error("oracle missing edge");
    }
    void link(int u,int v,int weight) {
        if(!path(u,v).empty())throw std::runtime_error("oracle cycle");
        adjacent[u].push_back({v,weight});
        adjacent[v].push_back({u,weight});
    }
    void cut(int u,int v) {
        auto remove=[&](int a,int b){
            auto& xs=adjacent[a];
            const auto old=xs.size();
            xs.erase(std::remove_if(xs.begin(),xs.end(),
                  [&](const auto& x){return x.first==b;}),xs.end());
            if(xs.size()+1!=old)throw std::runtime_error("oracle nonedge cut");
        };
        remove(u,v);remove(v,u);
    }
};
void require(bool ok,const std::string& explanation) {
    if(!ok)throw std::runtime_error(stage+": "+explanation);
}
void validate(Forest& forest,const Oracle& oracle) {
    forest.check_invariants();
    const int n=static_cast<int>(oracle.adjacent.size());
    for(int u=0;u<n;++u)for(int v=0;v<n;++v) {
        auto p=oracle.path(u,v);
        bool connected=!p.empty();
        require((forest.root(u)==forest.root(v))==connected,
                "connectivity mismatch "+std::to_string(u)+","+
                std::to_string(v));
        if(!connected)continue;
        const int actual_length=forest.path_length(u,v);
        if(actual_length!=static_cast<int>(p.size())) {
            throw std::runtime_error(stage+": path length mismatch "+
                std::to_string(u)+","+std::to_string(v)+
                " expected "+std::to_string(p.size())+
                " got "+std::to_string(actual_length)+
                "\nForest snapshot:\n"+forest.debug_state());
        }
        int best=1000000000;
        for(int j=0;j<static_cast<int>(p.size());++j) {
            require(forest.kth_on_path(u,v,j)==p[j],
                    "kth path mismatch "+std::to_string(u)+","+
                    std::to_string(v)+" index "+std::to_string(j));
            if(j)best=std::min(best,oracle.edge(p[j-1],p[j]));
        }
        require(forest.min_on_path(u,v)==best,
                "minimum edge mismatch "+std::to_string(u)+","+
                std::to_string(v));
    }
}
int main() {
    try {
        {
            Forest x(2); Oracle reference(2);
            stage="two-vertex initialization";
            x.check_invariants();
            x.link(0,1,7);reference.link(0,1,7);
            stage="two-vertex after link";validate(x,reference);
            x.evert(0);stage="two-vertex evert 0";validate(x,reference);
            x.evert(1);stage="two-vertex evert 1";validate(x,reference);
            x.cut(0,1);reference.cut(0,1);
            stage="two-vertex after cut";validate(x,reference);
            x.link(1,0,3);reference.link(1,0,3);
            stage="two-vertex after relink";validate(x,reference);
        }
        std::mt19937 rng(20261119);
        for(int n=3;n<=12;++n) {
            for(int trial=0;trial<30;++trial) {
                Forest forest(n);
                Oracle oracle(n);
                std::vector<std::string> history;
                for(int step=0;step<120;++step) {
                    const int u=static_cast<int>(rng()%n);
                    int v=static_cast<int>(rng()%n);
                    if(v==u) v=(v+1)%n;
                    if(n==8 && trial==0 && step==60)
                        std::cerr<<"BEFORE E2:\n"<<forest.debug_state();
                    const auto p=oracle.path(u,v);
                    std::string action;
                    if(p.empty()) {
                        const int weight=1+static_cast<int>(rng()%200);
                        action="L "+std::to_string(u)+" "+std::to_string(v)+" "+std::to_string(weight);
                        forest.link(u,v,weight);
                        oracle.link(u,v,weight);
                    } else if(p.size()==2) {
                        action="C "+std::to_string(u)+" "+std::to_string(v);
                        forest.cut(u,v);
                        oracle.cut(u,v);
                    } else {
                        action="E "+std::to_string(u);
                        forest.evert(u);
                    }
                    if(n==8 && trial==0 && step==60)
                        std::cerr<<"AFTER E2:\n"<<forest.debug_state();
                    history.push_back(action);
                    stage="n="+std::to_string(n)+" trial="+std::to_string(trial)+" step="+std::to_string(step)+" endpoints="+std::to_string(u)+","+std::to_string(v)+" action "+action;
                    if(step>=55) {
                        stage+="\nSequence:";
                        for(const auto& item:history)stage+="\n"+item;
                    }
                    validate(forest,oracle);
                }
            }
        }
        std::cout<<"PASS: ST1983 worst-case dynamic-forest functional tests\n";
    }catch(const std::exception& ex){
        std::cerr<<"FAIL at "<<stage<<": "<<ex.what()<<'\n';return 1;
    }
}
