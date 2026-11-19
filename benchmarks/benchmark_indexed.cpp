// Reproducible A1 versus indexed-path research-variant query experiment.
// The indexed variant is NOT the Sleator-Tarjan worst-case dynamic tree;
// it rebuilds a heavy-path index after each update.
#include "mtp/dynamic_ibarra.hpp"
#include "mtp/indexed_dynamic_ibarra.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <utility>

namespace {
using Clock=std::chrono::steady_clock;
double ms(Clock::time_point a,Clock::time_point b) {
    return std::chrono::duration<double,std::milli>(b-a).count();
}
mtp::Graph path(int n) {
    mtp::Graph graph(n);
    for(int v=1;v<n;++v)graph.add_edge(v-1,v);
    return graph;
}
}
int main() {
    std::cout << "n,m,baseline_build_ms,indexed_build_ms,"
                 "a1_insert_query_ms,indexed_insert_query_ms\n";
    std::cout << std::fixed << std::setprecision(7);
    volatile int sink=0;
    for(int n:{100,200,400,800,1600}) {
        const auto start=Clock::now();
        mtp::DynamicChordalGraph a1(path(n));
        const auto builtA1=Clock::now();
        mtp::IndexedDynamicChordalGraph indexed(path(n));
        const auto builtIndexed=Clock::now();
        const int trials=2000;
        const int u=0,v=n-1; // A distant nonedge in the path graph.
        const auto beforeA1=Clock::now();
        for(int i=0;i<trials;++i)sink+=a1.insert_query(u,v);
        const auto afterA1=Clock::now();
        for(int i=0;i<trials;++i)sink+=indexed.insert_query(u,v);
        const auto afterIndexed=Clock::now();
        std::cout << n << ',' << n-1 << ','
                  << ms(start,builtA1) << ',' << ms(builtA1,builtIndexed) << ','
                  << ms(beforeA1,afterA1)/trials << ','
                  << ms(afterA1,afterIndexed)/trials << '\n';
    }
    if(sink<0)std::cerr << sink << '\n';
}
