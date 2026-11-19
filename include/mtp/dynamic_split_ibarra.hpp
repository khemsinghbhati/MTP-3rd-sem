#pragma once
#include "mtp/graph.hpp"
#include <list>
#include <vector>

namespace mtp {
// Ibarra (2008), Section 8: Hammer-Simeone split degree-sequence test
// and constant-time graph update bookkeeping. Preprocessing O(n+m);
// query, insert and delete O(1) worst-case. Caller must provide valid
// operations (insert a nonedge, delete an existing edge): the degree
// sequence intentionally does not store the graph's edge set.
class DynamicSplitGraph {
public:
    explicit DynamicSplitGraph(const Graph& graph);
    [[nodiscard]] bool is_split() const noexcept;
    [[nodiscard]] int clique_number_when_split() const noexcept {return j_;}
    [[nodiscard]] int degree(int vertex) const;
    void insert_edge(int u,int v);
    void delete_edge(int u,int v);
private:
    struct Entry {int degree;bool left;};
    using Iterator=std::list<Entry>::iterator;
    std::list<Entry> sequence_;
    std::vector<Iterator> first_,last_;
    std::vector<bool> present_;
    std::vector<int> degree_;
    Iterator boundary_;
    int j_=0;
    long long sum_left_=0,sum_right_=0;
    void validate(int u,int v) const;
    void increase(Iterator it);
    void decrease(Iterator it);
    void update_boundary_after_insert();
    void update_boundary_after_delete();
};
} // namespace mtp
