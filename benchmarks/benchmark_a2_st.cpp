// Empirical comparison of the *actual published-method* Ibarra A1/A2.
// This executable measures observed wall-clock times, not Big-O proofs.
// All tests use the same chordal path graph; endpoints are not adjacent,
// and inserting their edge would create a long chordless cycle.
#include "mtp/dynamic_ibarra.hpp"
#include "mtp/dynamic_ibarra_a2.hpp"
#include "mtp/graph.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>

using Clock=std::chrono::steady_clock;
template<class Fn> double milliseconds(Fn&& fn) {
    const auto a=Clock::now();
    fn();
    const auto b=Clock::now();
    return std::chrono::duration<double,std::milli>(b-a).count();
}

int main() {
    std::cout<<std::fixed<<std::setprecision(8);
    std::cout<<"n,A1_construct_ms,A2_ST_construct_ms,A1_query_ms,A2_ST_query_ms,queries\n";
    constexpr int repetitions=700;
    volatile int sink=0;
    for(int n : {64,128,256,512,1024}) {
        mtp::Graph g(n);
        for(int i=1;i<n;++i)g.add_edge(i-1,i);
        double initA1=0,initA2=0;
        // Do not include construction time in the operation benchmark.
        const auto a=Clock::now();
        mtp::DynamicChordalGraph a1(g);
        const auto b=Clock::now();
        mtp::SleatorTarjanChordalGraphA2 a2(g);
        const auto c=Clock::now();
        initA1=std::chrono::duration<double,std::milli>(b-a).count();
        initA2=std::chrono::duration<double,std::milli>(c-b).count();
        if(a1.insert_query(0,n-1) || a2.insert_query(0,n-1))
            throw std::runtime_error("long path endpoint insertion incorrectly accepted");
        const double a1_ms=milliseconds([&]{
            for(int j=0;j<repetitions;++j)sink+=a1.insert_query(0,n-1);
        })/repetitions;
        const double a2_ms=milliseconds([&]{
            for(int j=0;j<repetitions;++j)sink+=a2.insert_query(0,n-1);
        })/repetitions;
        std::cout<<n<<','<<initA1<<','<<initA2<<','
                 <<a1_ms<<','<<a2_ms<<','<<repetitions<<'\n';
        a2.check_dynamic_tree_invariants();
    }
    (void)sink;
}
