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
                for(int i=0;i<n;++i) {
                    root=t.global_join(root,leaves[i]);
                    expected.push_back(i);
                    t.check_invariants(root);
                    require(t.sequence(root)==expected,"prefix join order mismatch");
                    ++tested;
                }
                // Repeatedly cut at arbitrary existing leaves and
                // rejoin using the original paper's global join.
                for(int iteration=0;iteration<200;++iteration) {
                    const int selected=static_cast<int>(rng()%n);
                    auto cuts=t.split(leaves[selected]);
                    std::vector<int> a(expected.begin(),expected.begin()+selected);
                    std::vector<int> b(expected.begin()+selected+1,expected.end());
                    t.check_invariants(cuts.before);
                    t.check_invariants(cuts.item);
                    t.check_invariants(cuts.after);
                    require(t.sequence(cuts.before)==a,"split prefix mismatch");
                    require(t.sequence(cuts.item)==std::vector<int>{selected},
                            "split selected item mismatch");
                    require(t.sequence(cuts.after)==b,"split suffix mismatch");
                    root=t.global_join(cuts.before,cuts.item);
                    root=t.global_join(root,cuts.after);
                    t.check_invariants(root);
                    require(t.sequence(root)==expected,"split/rejoin order mismatch");
                    ++tested;
                }
            }
        }
        std::cout<<"PASS: Bent–Sleator–Tarjan §4 global join/split tests: "
                 <<tested<<" verified operations\n";
    } catch(const std::exception& ex) {
        std::cerr<<"FAIL: "<<ex.what()<<'\n';
        return 1;
    }
}
