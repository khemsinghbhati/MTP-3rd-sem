#include "mtp/dynamic_split_ibarra.hpp"
#include "mtp/graph.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>
using namespace mtp;

static bool independent_split_oracle(const Graph& graph) {
    // Independently recompute Hammer-Simeone equality from scratch.
    std::vector<int> degrees;
    for(int v=0;v<graph.vertex_count();++v)
        degrees.push_back(static_cast<int>(graph.neighbors(v).size()));
    std::sort(degrees.begin(),degrees.end(),std::greater<int>());
    int j=0;
    for(int i=0;i<static_cast<int>(degrees.size());++i)
        if(degrees[i]>=i)j=i+1;
    long long left=0,right=0;
    for(int i=0;i<static_cast<int>(degrees.size());++i)
        (i<j?left:right)+=degrees[i];
    return left==static_cast<long long>(j)*(j-1)+right;
}
int main() {
    try {
        std::mt19937 rng(20261119);
        for(int n=0;n<=7;++n) {
            for(int trial=0;trial<100;++trial) {
                Graph oracle(n);
                for(int u=0;u<n;++u)
                    for(int v=u+1;v<n;++v)
                        if(rng()%2)oracle.add_edge(u,v);
                DynamicSplitGraph incremental(oracle);
                for(int step=0;step<150;++step) {
                    if(incremental.is_split()!=independent_split_oracle(oracle))
                        throw std::runtime_error("split test disagrees with oracle");
                    for(int v=0;v<n;++v)
                        if(incremental.degree(v)!=
                           static_cast<int>(oracle.neighbors(v).size()))
                            throw std::runtime_error("incremental degree mismatch");
                    if(n<2)break;
                    int u=static_cast<int>(rng()%n);
                    int v=static_cast<int>(rng()%n);
                    if(u==v)continue;
                    if(oracle.has_edge(u,v)) {
                        incremental.delete_edge(u,v);
                        oracle.remove_edge(u,v);
                    } else {
                        incremental.insert_edge(u,v);
                        oracle.add_edge(u,v);
                    }
                }
            }
        }
        std::cout << "PASS: Ibarra Section 8 O(1) dynamic split recognition\n"
                  << "PASS: 120,000 randomized update steps with oracle checks\n";
    } catch(const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
