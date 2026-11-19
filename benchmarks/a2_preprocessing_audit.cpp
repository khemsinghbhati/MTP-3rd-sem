// Ibarra 2008 Sections 4, 5.1 and 7 preprocessing / memory audit.
// This reads the implementation's exact dense storage dimensions:
// DynamicChordalGraph allocates (n+2) clique slots, each active clique
// owns an n-byte characteristic vector; A2 stores an (n+2)^2 int W.
// On an empty graph, k=n and m=0, hence both structures use Theta(n^2).
#include "mtp/dynamic_ibarra_a2.hpp"
#include "mtp/graph.hpp"
#include <chrono>
#include <cstddef>
#include <iostream>
#include <stdexcept>

int main() {
    std::cout<<"n,m,initial_maximal_cliques,char_vector_entries,W_entries,"
                "W_bytes,construct_ms\n";
    for(int n : {100,200,400,800,1600}) {
        mtp::Graph empty(n);
        const auto start=std::chrono::steady_clock::now();
        mtp::SleatorTarjanChordalGraphA2 a2(std::move(empty));
        const auto stop=std::chrono::steady_clock::now();
        if(a2.graph().vertex_count()!=n ||
           a2.graph().edge_count()!=0 ||
           static_cast<int>(a2.snapshot().cliques.size())!=n)
            throw std::runtime_error("empty graph initial clique invariant");
        const std::size_t char_entries=std::size_t(n)*std::size_t(n);
        const std::size_t W_entries=std::size_t(n+2)*std::size_t(n+2);
        const double milliseconds=std::chrono::duration<double,std::milli>(
            stop-start).count();
        std::cout<<n<<",0,"<<n<<','<<char_entries<<','<<W_entries<<','
                 <<W_entries*sizeof(int)<<','<<milliseconds<<'\n';
    }
}
