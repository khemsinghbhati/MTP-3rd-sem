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
        [[nodiscard]] bool is_leaf() const noexcept {return item>=0;}
    };
    struct Split {
        Node* before=nullptr;
        Node* item=nullptr;
        Node* after=nullptr;
    };

    // Each leaf must have a strictly positive integral weight.
    Node* leaf(int item, int weight);
    // Concatenation consumes roots (each must be detached).
    Node* global_join(Node* left, Node* right);
    // Split consumes the tree containing the leaf and returns three roots.
    Split split(Node* leaf);
    // Iterative linear-time independent validation of root-to-leaf order.
    [[nodiscard]] std::vector<int> sequence(Node* root) const;
    // Debug assertions: all four BST rank/bias conditions from Section 4.
    void check_invariants(Node* root) const;

private:
    std::vector<std::unique_ptr<Node>> storage_;
    std::vector<Node*> pool_;
    static int floor_log2(int weight);
    Node* internal(Node* left, Node* right, int rank);
    void set_left(Node* parent, Node* child);
    void set_right(Node* parent, Node* child);
    Node* tilt_left(Node* x);
    Node* tilt_right(Node* x);
    Node* local_join(Node* left, Node* right);
    void recycle(Node* internal_node);
};

} // namespace mtp
