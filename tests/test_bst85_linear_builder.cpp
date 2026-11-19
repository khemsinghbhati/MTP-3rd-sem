#include "mtp/globally_biased_binary_tree.hpp"
#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>
#include <string>
using BST=mtp::GloballyBiasedBinaryTree;
void require(bool p,const std::string& msg) {
    if(!p)throw std::runtime_error(msg);
}
int main(){
    try {
        std::mt19937 rng(1985);
        long long checked=0;
        for(int n : {1,2,3,4,5,6,7,8,16,32,64,128,256,512}) {
            for(int trial=0;trial<60;++trial) {
                BST tree;
                std::vector<BST::Node*> leaves;
                std::vector<int> boundary;
                for(int i=0;i<n;++i) {
                    const int weight=trial%4==0?1:
                                     trial%4==1?(1<<int(rng()%13)):
                                     trial%4==2?1+int(rng()%1000):
                                     (i%2==0?1:2048);
                    leaves.push_back(tree.leaf(i,weight));
                    if(i)boundary.push_back(int(rng()%2001)-1000);
                }
                BST::Node* root=tree.build_linear(leaves,boundary);
                tree.check_invariants(root);
                // BST85 Figure 15 O(n) proof invariant:
                // nodes on the rightmost spine strictly decrease in rank.
                for(auto* node=root;node && !node->is_leaf();
                    node=node->right) {
                    if(node->rank<=node->right->rank)
                        throw std::runtime_error("BST85 right-spine rank "
                            "invariant failed at n="+std::to_string(n)+
                            " trial="+std::to_string(trial));
                }
                require(root->leaf_count==n,"wrong number of leaves");
                require(tree.linear_build_steps()<=40LL*n,
                    "construction exceeds published linear work bound at n="+
                    std::to_string(n)+" steps="+
                    std::to_string(tree.linear_build_steps()));
                const auto ids=tree.sequence(root);
                for(int j=0;j<n;++j) {
                    require(ids[j]==j,"ordered leaf mismatch");
                    require(tree.kth(root,j)==leaves[j],
                            "kth-node mismatch");
                }
                const int reference=boundary.empty()?1000000000:
                    *std::min_element(boundary.begin(),boundary.end());
                require(tree.min_cost(root)==reference,
                        "path minimum mismatch");
                long long W=0;
                for(auto* p:leaves)W+=p->weight;
                require(root->sum_weight==W,
                        "path total weight mismatch");
                ++checked;
            }
        }
        std::cout<<"PASS: BST85 Section 4 linear builder: "<<checked
                 <<" weighted global-bias configurations with path aggregates\n";
    }catch(const std::exception& e){
        std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;
    }
}
