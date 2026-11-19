#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace mtp {

// Bent–Sleator–Tarjan (1985), "Biased Search Trees", Section 4,
// especially the globally biased binary trees and Figure 14.
//
// This is a research-stage translation of the paper's *actual*
// global-join, local-join, split algorithms. It is NOT a replacement
// dynamic tree, and is not wired into A2 until its invariants pass.
class GloballyBiasedBinaryTree {
public:
    struct Node {
        Node* parent=nullptr;
        Node* left=nullptr;
        Node* right=nullptr;
        int rank=0;
        int item=-1; // leaf ID, or -1 for an internal node
        int weight=0; // leaf weight; dynamic-tree path weights
        bool live=true;
        // Sleator–Tarjan (1983), §4–5, path aggregate fields.
        int edge_cost=0; // internal edge between the rightmost left and leftmost right leaves
        int minimum_cost=1000000000;
        long long sum_weight=0;
        long long left_tilt_min=1000000000000LL;
        long long right_tilt_min=1000000000000LL;
        Node* first=nullptr;
        Node* last=nullptr;
        int leaf_count=0;
        bool reversed=false;
        [[nodiscard]] bool is_leaf() const noexcept {return item>=0;}
    };
    struct Split {
        Node* before=nullptr;
        Node* item=nullptr;
        Node* after=nullptr;
        int before_cost=0;
        int after_cost=0;
    };

    // Each leaf must have a strictly positive integral weight.
    Node* leaf(int item, int weight);
    // The ST expose/conceal steps change the weight of a detached leaf.
    void set_detached_leaf_weight(Node* leaf, int new_weight);
    // Recycle a detached path-set leaf; main-tree vertex leaves persist.
    void release_detached_leaf(Node* leaf);
    // Concatenation consumes roots (each must be detached).
    Node* global_join(Node* left, Node* right, int edge_cost=0);
    // Bent–Sleator–Tarjan (1985), Section 4, Figure 15:
    // bottom-up globally biased tree construction in O(k) WORST CASE
    // over k input leaves (their preexisting order is preserved).
    // boundary_cost[i] labels the edge between leaves i and i+1.
    Node* build_linear(const std::vector<Node*>& leaves,
                       const std::vector<int>& boundary_cost);
    [[nodiscard]] long long linear_build_steps() const noexcept {
        return linear_steps_;
    }
    // Dynamic path operations (Sleator–Tarjan 1983, §§3–5).
    void reverse(Node* root);
    [[nodiscard]] Node* kth(Node* root, int position) const;
    [[nodiscard]] int min_cost(Node* root) const;
    [[nodiscard]] Node* light(Node* root) const;
    // Split consumes the tree containing the leaf and returns three roots.
    Split split(Node* leaf);
    // Iterative linear-time independent validation of root-to-leaf order.
    [[nodiscard]] std::vector<int> sequence(Node* root) const;
    // Debug assertions: all four BST rank/bias conditions from Section 4.
    void check_invariants(Node* root) const;

private:
    std::vector<std::unique_ptr<Node>> storage_;
    std::vector<Node*> pool_;
    std::vector<Node*> leaf_pool_;
    long long linear_steps_=0;
    static int floor_log2(int weight);
    Node* internal(Node* left, Node* right, int rank, int edge_cost);
    void refresh(Node* x);
    static void flip(Node* x);
    static void push(Node* x);
    void set_left(Node* parent, Node* child);
    void set_right(Node* parent, Node* child);
    Node* tilt_left(Node* x);
    Node* tilt_right(Node* x);
    Node* local_join(Node* left, Node* right, int edge_cost);
    void recycle(Node* internal_node);
};

} // namespace mtp
