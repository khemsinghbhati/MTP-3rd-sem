#include "mtp/dynamic_ibarra.hpp"
#include "mtp/graph.hpp"
#include <iostream>
#include <stdexcept>

using namespace mtp;
int main() {
    try {
        // Sparse stars: many small maximal cliques and no overflow members.
        for (int n: {128,256}) {
            Graph graph(n);
            for (int v=1;v<n;++v)graph.add_edge(0,v);
            DynamicChordalGraph dynamic(std::move(graph));
            dynamic.enable_fast_delete_queries();
            for (int t=0;t<30;++t) {
                const int u=1+(t%(n-2)),v=u+1;
                if(!dynamic.insert_query(u,v)||
                   !dynamic.insert_edge(u,v)||
                   !dynamic.delete_query(u,v)||
                   !dynamic.delete_edge(u,v)||
                   dynamic.delete_query(u,v)||
                   !dynamic.delete_query(0,v))
                    throw std::runtime_error("star fast-index regression");
            }
        }
        // Complete graphs: the unique large clique has overflow members.
        // Repeated legal edge deletions and insertions exercise both
        // clique splitting/contraction and COMMON/overflow maintenance.
        for (int n: {100,200}) {
            Graph graph(n);
            for (int u=0;u<n;++u)
                for (int v=u+1;v<n;++v)
                    graph.add_edge(u,v);
            DynamicChordalGraph dynamic(std::move(graph));
            dynamic.enable_fast_delete_queries();
            for (int t=0;t<40;++t) {
                const int u=t%n,v=(t+1)%n;
                if(!dynamic.delete_query(u,v)||
                   !dynamic.delete_edge(u,v)||
                   dynamic.delete_query(u,v)||
                   !dynamic.insert_query(u,v)||
                   !dynamic.insert_edge(u,v)||
                   !dynamic.delete_query(u,v))
                    throw std::runtime_error("complete-graph fast-index regression");
            }
        }
        std::cout << "PASS: modified Yellin index on sparse stars "
                     "and large dense cliques under legal updates\n";
    }catch(const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
