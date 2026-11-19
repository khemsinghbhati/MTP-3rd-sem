#pragma once
#include <list>
#include <vector>

namespace mtp {
// Ibarra (2008), Sections 6.2-6.3, modified Yellin set-intersection
// structure. Each maximal clique element x partitions its vertex set
// into x.member and x.overflow. COMMON counts member-only pairs.
// Intersect query O(sqrt(m)+1) worst case for m current graph edges.
// Constant number of changed cliques + threshold rebalancing O(m+n).
class FastCliqueIntersectionIndex {
public:
    FastCliqueIntersectionIndex(int vertex_count,int edge_count);
    void add_clique(int clique_id,const std::vector<unsigned char>& bits);
    void remove_clique(int clique_id);
    void rebalance(int edge_count);
    [[nodiscard]] int count_common_cliques(int u,int v) const;
private:
    using Iterator=std::list<int>::iterator;
    struct Element {
        bool active=false,in_overflows=false;
        std::list<int> member,overflow;
        std::vector<unsigned char> member_bit,overflow_bit;
        std::vector<Iterator> member_iterator,overflow_iterator;
        Iterator overflow_global_iterator;
    };
    int n_=0,threshold_=1;
    std::vector<Element> elements_;
    std::vector<std::vector<int>> common_;
    std::list<int> overflows_;
    void add_member(int element,int vertex);
    void remove_member(int element,int vertex);
    void add_overflow(int element,int vertex);
    void remove_overflow(int element,int vertex);
    static int bound(int edge_count);
};
} // namespace mtp
