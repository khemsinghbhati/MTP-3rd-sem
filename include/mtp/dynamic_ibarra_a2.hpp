#pragma once

#include "mtp/dynamic_ibarra.hpp"
#include "mtp/worst_case_dynamic_tree.hpp"

#include <utility>
#include <vector>

namespace mtp {

// Ibarra (2008), Sections 3–5.1: A2 SOURCE-METHOD RESEARCH INTEGRATION.
// Dynamic forest: Sleator–Tarjan (1983), §5 worst-case globally biased
// partition by size. Clique intersections: Ibarra §5.1 matrix W.
//
// This is not accepted as the final paper A2 until correctness AND
// the exact worst-case insertion bound O(n) are independently verified.
class SleatorTarjanChordalGraphA2 {
public:
    explicit SleatorTarjanChordalGraphA2(Graph graph);
    [[nodiscard]] bool insert_query(int u,int v);
    [[nodiscard]] bool delete_query(int u,int v) const;
    bool insert_edge(int u,int v);
    bool delete_edge(int u,int v);
    void enable_fast_delete_queries();
    [[nodiscard]] CliqueTree snapshot() const;
    [[nodiscard]] const Graph& graph() const noexcept;
    void check_dynamic_tree_invariants();
    [[nodiscard]] int dynamic_tree_link_count_last_update() const noexcept {
        return link_count_last_;
    }
    [[nodiscard]] int dynamic_tree_cut_count_last_update() const noexcept {
        return cut_count_last_;
    }
    // Explicit accounting: high-degree contraction invokes the
    // published O(n) static ST builder instead of Theta(n) log-time
    // link/cut calls. Counted separately from incremental ST calls.
    [[nodiscard]] bool rebuilt_dynamic_forest_last_insert() const noexcept {
        return rebuilt_last_insert_;
    }
    [[nodiscard]] long long linear_rebuild_steps_last_insert() const noexcept {
        return rebuild_steps_last_insert_;
    }

private:
    DynamicChordalGraph data_;
    SleatorTarjanWorstCaseForest tree_;
    std::vector<int> vertex_witness_;
    std::vector<std::vector<int>> W_;
    int link_count_last_=0,cut_count_last_=0;
    bool rebuilt_last_insert_=false;
    long long rebuild_steps_last_insert_=0;
    [[nodiscard]] std::pair<int,int> closest(int u,int v);
    void initialize_weights();
    void attach_tree_hooks();
};

} // namespace mtp
