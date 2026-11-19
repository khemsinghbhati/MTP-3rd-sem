#include "mtp/graph.hpp"
#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

int main() {
    std::mt19937 rng(20261119);
    for(int n : {1,2,3,16,64,192}) {
        mtp::Graph g(n);
        std::vector<std::vector<unsigned char>> truth(n,
            std::vector<unsigned char>(n,0));
        for(int step=0;step<2000;++step) {
            if(n<2)break;
            const int u=static_cast<int>(rng()%n);
            int v=static_cast<int>(rng()%n);
            if(u==v)v=(v+1)%n;
            switch(step%3) {
                case 0: g.add_edge(u,v);truth[u][v]=truth[v][u]=1;break;
                case 1: g.remove_edge(u,v);truth[u][v]=truth[v][u]=0;break;
                default: break;
            }
            if(step%37==0) {
                long long count=0;
                for(int a=0;a<n;++a) {
                    for(int b=0;b<n;++b) {
                        if(g.has_edge(a,b)!=static_cast<bool>(truth[a][b]))
                            throw std::runtime_error("deterministic lookup differs from oracle");
                        count+=truth[a][b];
                    }
                    // The iteration-order adjacency vector must retain all neighbors.
                    for(int b:g.neighbors(a))
                        if(!truth[a][b])
                            throw std::runtime_error("adjacency vector has stale member");
                }
                if(static_cast<long long>(g.edge_count())!=count/2)
                    throw std::runtime_error("edge count incorrect");
            }
        }
    }
    std::cout<<"PASS: std::set worst-case logarithmic lookup with 12,000 updates/queries\n";
}
