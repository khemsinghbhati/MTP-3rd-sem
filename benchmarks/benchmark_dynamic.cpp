// Reproducible timing harness for Ibarra (2008), Section 4.
// This is measurement code only; not a substitute for asymptotic proofs.
// Build with -DMTP_BUILD_BENCHMARKS=ON; run ./build/mtp_benchmarks.
#include "mtp/dynamic_ibarra.hpp"
#include "mtp/graph.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>

namespace {
using Clock = std::chrono::steady_clock;
double milliseconds(Clock::time_point start, Clock::time_point finish) {
    return std::chrono::duration<double, std::milli>(finish-start).count();
}
mtp::Graph star(int n) {
    mtp::Graph graph(n);
    for (int v=1;v<n;++v) graph.add_edge(0,v);
    return graph;
}
}
int main() {
    std::cout << "n,m,build_ms,insert_query_ms,insert_update_ms,"
                 "delete_query_ms,delete_update_ms\n";
    std::cout << std::fixed << std::setprecision(5);
    for (int n : {50, 100, 200, 400, 800, 1600}) {
        auto initial = star(n);
        const auto start = Clock::now();
        mtp::DynamicChordalGraph dynamic(std::move(initial));
        const auto built = Clock::now();
        double iq=0.0, iu=0.0, dq=0.0, du=0.0;
        // Each pair of distinct leaves is a nonedge in a star.
        // Insertion creates a triangle; removing that edge restores
        // the original star. Both updates preserve chordality.
        const int trials = 200;
        for (int i=0;i<trials;++i) {
            const int u = 1 + (i % (n-1));
            const int v = 1 + ((i+1) % (n-1));
            const auto a = Clock::now();
            const bool can_insert = dynamic.insert_query(u,v);
            const auto b = Clock::now();
            const bool inserted = can_insert && dynamic.insert_edge(u,v);
            const auto c = Clock::now();
            const bool can_delete = inserted && dynamic.delete_query(u,v);
            const auto d = Clock::now();
            const bool deleted = can_delete && dynamic.delete_edge(u,v);
            const auto e = Clock::now();
            if (!deleted) {
                std::cerr << "Chordality-preserving benchmark step rejected\n";
                return 1;
            }
            iq += milliseconds(a,b);
            iu += milliseconds(b,c);
            dq += milliseconds(c,d);
            du += milliseconds(d,e);
        }
        std::cout << n << ',' << n-1 << ',' << milliseconds(start,built)
                  << ',' << iq/trials << ',' << iu/trials
                  << ',' << dq/trials << ',' << du/trials << '\n';
    }
}
