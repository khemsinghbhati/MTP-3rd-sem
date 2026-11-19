#include "mtp/globally_biased_binary_tree.hpp"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

using Tree=mtp::GloballyBiasedBinaryTree;
void require(bool condition,const char* message) {
    if(!condition)throw std::runtime_error(message);
}
int main() {
    try {
        std::mt19937 rng(20261119);
        long long tested=0;
        for(int n=1;n<=64;++n) {
            for(int trial=0;trial<60;++trial) {
                Tree t;
                std::vector<Tree::Node*> leaves;
                leaves.reserve(n);
                for(int i=0;i<n;++i) {
                    // Mix equal ranks, very unbalanced weights, and
                    // weights spanning at least nine binary ranks.
                    int w=(trial%3==0)?1:
                          (trial%3==1)?1+static_cast<int>(rng()%1000):
                          (1<<static_cast<int>(rng()%10));
                    leaves.push_back(t.leaf(i,w));
                }
                Tree::Node* root=nullptr;
                std::vector<int> expected;
                std::vector<int> edge_costs;
                long long total_weight=0;
                for(int i=0;i<n;++i) {
                    int cost=-1000+static_cast<int>(rng()%2001);
                    if(i>0)edge_costs.push_back(cost);
                    root=t.global_join(root,leaves[i],cost);
                    expected.push_back(i);
                    total_weight+=leaves[i]->weight;
                    t.check_invariants(root);
                    require(t.sequence(root)==expected,"prefix join order mismatch");
                    require(root->sum_weight==total_weight,"aggregate subtree weight");
                    require(t.min_cost(root)==
                        (edge_costs.empty()?1000000000:
                         *std::min_element(edge_costs.begin(),edge_costs.end())),
                        "prefix minimum edge cost incorrect");
                    require(t.kth(root,i)==leaves[i],"kth prefix node");
                    ++tested;
                }
                // Repeatedly cut at arbitrary existing leaves and
                // rejoin using the original paper's global join.
                for(int iteration=0;iteration<200;++iteration) {
                    const int selected=static_cast<int>(rng()%n);
                    auto cuts=t.split(leaves[selected]);
                    require(cuts.before_cost==(selected>0?edge_costs[selected-1]:0),
                            "split left boundary edge cost");
                    require(cuts.after_cost==(selected+1<n?edge_costs[selected]:0),
                            "split right boundary edge cost");
                    std::vector<int> a(expected.begin(),expected.begin()+selected);
                    std::vector<int> b(expected.begin()+selected+1,expected.end());
                    t.check_invariants(cuts.before);
                    t.check_invariants(cuts.item);
                    t.check_invariants(cuts.after);
                    require(t.sequence(cuts.before)==a,"split prefix mismatch");
                    require(t.sequence(cuts.item)==std::vector<int>{selected},
                            "split selected item mismatch");
                    require(t.sequence(cuts.after)==b,"split suffix mismatch");
                    root=t.global_join(cuts.before,cuts.item,cuts.before_cost);
                    root=t.global_join(root,cuts.after,cuts.after_cost);
                    t.check_invariants(root);
                    require(t.sequence(root)==expected,"split/rejoin order mismatch");
                    require(root->sum_weight==total_weight,"rejoined subtree weight");
                    require(t.min_cost(root)==
                        (edge_costs.empty()?1000000000:
                         *std::min_element(edge_costs.begin(),edge_costs.end())),
                        "rejoined path minimum incorrect");
                    for(int j=0;j<n;++j)
                        require(t.kth(root,j)==leaves[j],"kth after split/rejoin");
                    ++tested;
                }
                // Independent direct root-to-tail light-edge computation.
                long long prefix=leaves[0]->weight;
                int expected_light=-1;
                for(int i=1;i<n;++i) {
                    if(prefix<=leaves[i]->weight)expected_light=i;
                    prefix+=leaves[i]->weight;
                }
                auto* light=t.light(root);
                require((light?light->item:-1)==expected_light,
                        "light edge weight criterion");
                t.reverse(root);
                auto reversed=expected;
                std::reverse(reversed.begin(),reversed.end());
                require(t.sequence(root)==reversed,
                        "path reverse order incorrect");
                require(t.min_cost(root)==
                    (edge_costs.empty()?1000000000:
                     *std::min_element(edge_costs.begin(),edge_costs.end())),
                    "reverse changed edge minimum");
                t.reverse(root);
                require(t.sequence(root)==expected,
                        "restored path reverse order incorrect");
                t.check_invariants(root);
            }
        }
        std::cout<<"PASS: Bent–Sleator–Tarjan §4 global join/split tests: "
                 <<tested<<" verified operations\n";
    } catch(const std::exception& ex) {
        std::cerr<<"FAIL: "<<ex.what()<<'\n';
        return 1;
    }
}
