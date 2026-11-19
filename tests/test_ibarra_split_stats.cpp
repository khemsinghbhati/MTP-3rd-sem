#include "mtp/dynamic_split_ibarra.hpp"
#include "mtp/graph.hpp"
#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {
using namespace mtp;
std::pair<int,int> brute_omega_alpha(const Graph& graph) {
    const int n=graph.vertex_count();
    int maximum_clique=0,maximum_independent=0;
    for (int mask=0;mask<(1<<n);++mask) {
        bool clique=true,independent=true;
        int count=0;
        for (int u=0;u<n;++u) {
            if (!(mask & (1<<u)))continue;
            ++count;
            for (int v=u+1;v<n;++v) {
                if (!(mask & (1<<v)))continue;
                if (!graph.has_edge(u,v))clique=false;
                if (graph.has_edge(u,v))independent=false;
            }
        }
        if (clique) maximum_clique=std::max(maximum_clique,count);
        if (independent) maximum_independent=std::max(maximum_independent,count);
    }
    return {maximum_clique,maximum_independent};
}
}
int main() {
    try {
        std::mt19937 random(20261119);
        for (int n=0;n<=7;++n) {
            for (int trial=0;trial<80;++trial) {
                Graph graph(n);
                for (int u=0;u<n;++u)
                    for (int v=u+1;v<n;++v)
                        if (random()%2)graph.add_edge(u,v);
                DynamicSplitStatistics data(graph);
                for (int step=0;step<120;++step) {
                    if (data.is_split()) {
                        const auto [omega,alpha]=brute_omega_alpha(graph);
                        if (data.clique_number()!=omega ||
                            data.chromatic_number()!=omega ||
                            data.independence_number()!=alpha ||
                            data.clique_cover_number()!=alpha)
                            throw std::runtime_error(
                                "one of four split numbers differs from oracle");
                    }
                    if (n<2)break;
                    const int u=static_cast<int>(random()%n);
                    const int v=static_cast<int>(random()%n);
                    if (u==v)continue;
                    if (graph.has_edge(u,v)) {
                        data.delete_edge(u,v);
                        graph.remove_edge(u,v);
                    } else {
                        data.insert_edge(u,v);
                        graph.add_edge(u,v);
                    }
                }
            }
        }
        std::cout << "PASS: split clique/chromatic/independence/cover invariants\n";
    }catch(const std::exception& e){
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
