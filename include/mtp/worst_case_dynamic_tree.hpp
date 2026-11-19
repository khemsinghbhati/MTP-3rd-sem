#pragma once
#include "mtp/globally_biased_binary_tree.hpp"
#include <vector>
#include <string>

namespace mtp {

// Sleator–Tarjan (1983), "A Data Structure for Dynamic Trees":
// Section 5, globally biased binary paths and path sets, with the
// published expose/conceal/splice/slice operations.
//
// RESEARCH STAGING: This class is not approved for the production Ibarra
// A2 until the published worst-case guarantees and structural invariants
// are independently verified on dynamic operation sequences.
class SleatorTarjanWorstCaseForest {
public:
    explicit SleatorTarjanWorstCaseForest(int vertex_count);
    struct Edge {
        int u=0,v=0,cost=0;
    };
    // Published-source static construction: ST 1983 §§4–5 size-based
    // partition, BST 1985 §4 Figure 15 linear biased tree build.
    // Takes O(n) worst-case for a verified n-vertex input FOREST
    // with <=n-1 edges; the represented structure remains precisely
    // the ST worst-case dynamic forest.
    static SleatorTarjanWorstCaseForest build_linear_forest(
        int vertex_count,const std::vector<Edge>& edges);
    [[nodiscard]] long long static_build_steps() const noexcept {
        return static_steps_;
    }
    void link(int u,int v,int weight);
    void cut(int u,int v);
    void evert(int u);
    [[nodiscard]] int root(int u);
    [[nodiscard]] int min_on_path(int u,int v);
    // Index is zero-based from u towards v, endpoints included.
    [[nodiscard]] int kth_on_path(int u,int v,int index);
    [[nodiscard]] int path_length(int u,int v);
    void check_invariants();
    [[nodiscard]] std::string debug_state() const;

private:
    using BST=GloballyBiasedBinaryTree;
    using Node=BST::Node;
    BST paths_,sets_;
    long long static_steps_=0;
    void initialize_linear_forest(const std::vector<Edge>& edges);
    std::vector<Node*> vertex_,path_set_root_,path_entry_,path_root_;
    // Path-root pointer indexed by its tail: ST1983 Section 5 maxwt O(1).
    std::vector<int> dashed_parent_,dashed_cost_,entry_owner_;
    [[nodiscard]] Node* path(int u) const;
    [[nodiscard]] static int first(Node* p);
    [[nodiscard]] static int tail(Node* p);
    [[nodiscard]] static int weight(Node* p);
    void insert_path_set(Node* p,int v);
    void delete_path_set(Node* p,int v);
    [[nodiscard]] Node* max_path(int v) const;
    [[nodiscard]] BST::Split split_vertex(int v);
    [[nodiscard]] Node* expose(int u);
    [[nodiscard]] Node* splice(Node* p);
    [[nodiscard]] Node* slice(Node* p);
    void conceal(Node* p);
    [[nodiscard]] Node* concatenate(Node* p,Node* q,int cost);
    void verify_vertex(int u) const;
};

} // namespace mtp
