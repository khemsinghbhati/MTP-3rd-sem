#include "mtp/dynamic_ibarra_a2.hpp"
#include <iostream>
#include <random>
#include <stdexcept>
using mtp::Graph;
using mtp::DynamicChordalGraph;
using mtp::SleatorTarjanChordalGraphA2;
void require(bool p,const std::string& message) {
    if(!p)throw std::runtime_error(message);
}
int main(){
    try{
        std::mt19937 rng(20261120);
        std::cout<<"n,rebuild_steps,st_link_calls,st_cut_calls\n";
        for(int k:{2,4,8,16,32,64,128}) {
            const int n=2*k+3;
            Graph graph(n);
            graph.add_edge(0,1);
            graph.add_edge(0,2);
            for(int i=0;i<k;++i){
                graph.add_edge(1,3+i);
                graph.add_edge(2,3+k+i);
            }
            DynamicChordalGraph baseline(graph);
            SleatorTarjanChordalGraphA2 a2(graph);
            if(k%2==0)a2.enable_fast_delete_queries();
            for(int cycle=0;cycle<3;++cycle){
                require(baseline.insert_query(1,2),"A1 should permit insertion");
                require(a2.insert_query(1,2),"A2 should permit insertion");
                require(baseline.insert_edge(1,2),"A1 insertion rejected");
                require(a2.insert_edge(1,2),"A2 insertion rejected");
                require(a2.rebuilt_dynamic_forest_last_insert(),
                    "both-subsumed insertion did not use BST85 O(n) builder");
                const auto count=a2.linear_rebuild_steps_last_insert();
                if(count>100LL*(n+2))
                    throw std::runtime_error("ST static build too expensive n="+
                        std::to_string(n)+" steps="+std::to_string(count));
                require(a2.dynamic_tree_link_count_last_update()==0 &&
                        a2.dynamic_tree_cut_count_last_update()==0,
                        "dual-clique contraction still uses per-edge ST updates");
                a2.check_dynamic_tree_invariants();
                for(int trial=0;trial<80;++trial){
                    int u=int(rng()%n),v=int(rng()%n);
                    if(u==v)continue;
                    require(a2.insert_query(u,v)==baseline.insert_query(u,v),
                            "post-rebuild insert query mismatch");
                    require(a2.delete_query(u,v)==baseline.delete_query(u,v),
                            "post-rebuild deletion query mismatch");
                }
                if(cycle==0)std::cout<<n<<','<<count<<','
                    <<a2.dynamic_tree_link_count_last_update()<<','
                    <<a2.dynamic_tree_cut_count_last_update()<<'\n';
                require(a2.delete_query(1,2)==baseline.delete_query(1,2),
                        "source edge deletion permission mismatch");
                require(a2.delete_edge(1,2)&&baseline.delete_edge(1,2),
                        "source deletion failed after linear rebuild");
                a2.check_dynamic_tree_invariants();
            }
        }
        std::cout<<"PASS: three insertion/deletion cycles on seven high-degree"
                    " two-hub graphs, exact ST linear rebuilding and A1 oracles\n";
    }catch(const std::exception& e) {
        std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;
    }
}
