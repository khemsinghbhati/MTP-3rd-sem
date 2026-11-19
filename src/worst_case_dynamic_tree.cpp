#include "mtp/worst_case_dynamic_tree.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <sstream>
#include <unordered_set>
#include <utility>
#include <vector>

namespace mtp {

SleatorTarjanWorstCaseForest::SleatorTarjanWorstCaseForest(int n)
    : vertex_(n), path_set_root_(n,nullptr), path_entry_(n,nullptr),
      path_root_(n,nullptr),
      dashed_parent_(n,-1), dashed_cost_(n,0), entry_owner_(n,-1) {
    if(n<0)throw std::invalid_argument("negative node count");
    for(int v=0;v<n;++v) {
        vertex_[v]=paths_.leaf(v,1);
        path_root_[v]=vertex_[v];
    }
}

// Static construction of THE SAME Sleator–Tarjan (1983) Section 5
// worst-case dynamic-tree representation, not an HLD/splay substitute.
// ST1983 §4: heavy edge iff 2*size(child)>size(parent).
// ST1983 §5: path vertex weight = size(v)-size(heavy child),
// and dashed paths are stored in globally biased path sets sorted by
// decreasing path weight.
// Bent–Sleator–Tarjan (1985) §4, Fig.15, Theorem 12 establishes
// linear-time static construction of each globally biased binary tree.
// The total number of leaves across ALL solid-path BSTs is n;
// across ALL dashed path-set BSTs it is <n. Count sort their
// integer weights in [1,n] to achieve total O(n) initialization.
SleatorTarjanWorstCaseForest
SleatorTarjanWorstCaseForest::build_linear_forest(
    int n,const std::vector<Edge>& edges) {
    SleatorTarjanWorstCaseForest result(n);
    result.initialize_linear_forest(edges);
    return result;
}
void SleatorTarjanWorstCaseForest::initialize_linear_forest(
    const std::vector<Edge>& edges) {
    const int n=static_cast<int>(vertex_.size());
    if(edges.size()>static_cast<std::size_t>(n>0?n-1:0))
        throw std::invalid_argument("n-vertex forest has at most n-1 edges");
    std::vector<std::vector<std::pair<int,int>>> adjacency(n);
    for(const auto& edge:edges) {
        verify_vertex(edge.u);verify_vertex(edge.v);
        if(edge.u==edge.v)
            throw std::invalid_argument("self edge in static ST forest");
        adjacency[edge.u].push_back({edge.v,edge.cost});
        adjacency[edge.v].push_back({edge.u,edge.cost});
    }
    std::vector<int> parent(n,-2),parent_cost(n,0),order;
    order.reserve(n);
    for(int r=0;r<n;++r) {
        if(parent[r]!=-2)continue;
        parent[r]=-1;
        std::vector<int> stack{r};
        while(!stack.empty()) {
            const int u=stack.back();stack.pop_back();
            order.push_back(u);
            for(const auto& [v,c]:adjacency[u]) {
                if(v==parent[u])continue;
                if(parent[v]!=-2)
                    throw std::invalid_argument("input is not a forest");
                parent[v]=u;parent_cost[v]=c;
                stack.push_back(v);
            }
        }
    }

    // A standard reverse DFS computes subtree sizes and the
    // size-partition of the rooted underlying clique forest.
    std::vector<int> subtree(n,1),heavy(n,-1);
    for(auto it=order.rbegin();it!=order.rend();++it) {
        const int v=*it;
        if(parent[v]>=0)subtree[parent[v]]+=subtree[v];
    }
    for(int u=0;u<n;++u)
        for(const auto& [v,c]:adjacency[u]) {
            (void)c;
            if(parent[v]!=u)continue;
            if(2LL*subtree[v]>subtree[u]) {
                if(heavy[u]!=-1)
                    throw std::logic_error("two heavy children");
                heavy[u]=v;
            }
        }

    for(int v=0;v<n;++v) {
        const int weight=subtree[v]-
                         (heavy[v]>=0?subtree[heavy[v]]:0);
        paths_.set_detached_leaf_weight(vertex_[v],weight);
    }
    // Form each maximal heavy path in bottom-to-top order, with
    // exactly one globally biased BST leaf per original vertex.
    // A path begins at a vertex with no heavy child.
    std::vector<int> path_tails;
    path_tails.reserve(n);
    for(int bottom=0;bottom<n;++bottom) {
        if(heavy[bottom]>=0)continue;
        std::vector<Node*> leaves;
        std::vector<int> costs;
        int u=bottom;
        leaves.push_back(vertex_[u]);
        while(parent[u]>=0 && heavy[parent[u]]==u) {
            costs.push_back(parent_cost[u]);
            u=parent[u];
            leaves.push_back(vertex_[u]);
        }
        Node* p=paths_.build_linear(leaves,costs);
        static_steps_+=paths_.linear_build_steps();
        const int top=u;
        path_root_[top]=p;
        path_tails.push_back(top);
        if(parent[top]>=0) {
            dashed_parent_[top]=parent[top];
            dashed_cost_[top]=parent_cost[top];
        }
    }

    // Counting-sort all dashed paths by size, descending.
    // ST83 §5 path-set invariant requires descending path weight,
    // with arbitrary order permitted among ties.
    std::vector<std::vector<int>> buckets(n+1),children(n);
    for(int tail: path_tails) {
        const int owner=dashed_parent_[tail];
        if(owner<0)continue;
        const int path_weight=weight(path_root_[tail]);
        if(path_weight<1 || path_weight>n)
            throw std::logic_error("invalid ST path size");
        buckets[path_weight].push_back(tail);
    }
    for(int w=n;w>=1;--w)
        for(int tail:buckets[w])
            children[dashed_parent_[tail]].push_back(tail);

    // BST85 Theorem 12 applies independently to each path-set tree.
    // The total number of path-set leaves is at most n-1.
    for(int owner=0;owner<n;++owner) {
        if(children[owner].empty())continue;
        std::vector<Node*> leaves;
        leaves.reserve(children[owner].size());
        for(int tail:children[owner]) {
            Node* p=path_root_[tail];
            if(!p||p->last->item!=tail)
                throw std::logic_error("static ST dashed path cache");
            Node* entry=sets_.leaf(tail,weight(p));
            path_entry_[tail]=entry;
            entry_owner_[tail]=owner;
            leaves.push_back(entry);
        }
        std::vector<int> costs(leaves.size()-1,0);
        path_set_root_[owner]=sets_.build_linear(leaves,costs);
        static_steps_+=sets_.linear_build_steps();
    }
    static_steps_+=6LL*n+2LL*edges.size();
}

void SleatorTarjanWorstCaseForest::verify_vertex(int u) const {
    if(u<0||u>=static_cast<int>(vertex_.size()))
        throw std::out_of_range("ST vertex outside forest");
}
SleatorTarjanWorstCaseForest::Node*
SleatorTarjanWorstCaseForest::path(int u) const {
    verify_vertex(u);
    Node* p=vertex_[u];
    while(p->parent)p=p->parent;
    return p;
}
int SleatorTarjanWorstCaseForest::first(Node* p) {
    if(!p)throw std::logic_error("empty solid path");
    return p->first->item;
}
int SleatorTarjanWorstCaseForest::tail(Node* p) {
    if(!p)throw std::logic_error("empty solid path");
    return p->last->item;
}
int SleatorTarjanWorstCaseForest::weight(Node* p) {
    if(!p)return 0;
    if(p->sum_weight>1000000000LL)
        throw std::overflow_error("path weight outside supported range");
    return static_cast<int>(p->sum_weight);
}
SleatorTarjanWorstCaseForest::BST::Split
SleatorTarjanWorstCaseForest::split_vertex(int v) {
    auto pieces=paths_.split(vertex_[v]);
    if(pieces.before)path_root_[tail(pieces.before)]=pieces.before;
    path_root_[v]=pieces.item;
    if(pieces.after)path_root_[tail(pieces.after)]=pieces.after;
    return pieces;
}
SleatorTarjanWorstCaseForest::Node*
SleatorTarjanWorstCaseForest::concatenate(Node* p,Node* q,int c) {
    Node* merged=paths_.global_join(p,q,c);
    path_root_[tail(merged)]=merged;
    return merged;
}

void SleatorTarjanWorstCaseForest::insert_path_set(Node* p,int v) {
    // Sleator–Tarjan (1983), Section 5: dashed paths in decreasing
    // path-weight order, represented by a globally biased search tree.
    const int t=tail(p);
    if(dashed_parent_[t]!=v||path_entry_[t])
        throw std::logic_error("dashed path already in a path set");
    Node* element=sets_.leaf(t,weight(p));
    auto less=[](Node* a,Node* b) {
        if(a->weight!=b->weight)return a->weight>b->weight;
        return a->item<b->item;
    };
    Node* root=path_set_root_[v];
    if(!root) {
        path_set_root_[v]=element;
    } else {
        Node* at=root;
        while(!at->is_leaf()) {
            // Keys at each boundary are maintained by root.first.
            if(less(element,at->right->first))at=at->left;
            else at=at->right;
        }
        auto pieces=sets_.split(at);
        Node* result;
        if(less(element,at)) {
            result=sets_.global_join(pieces.before,element);
            result=sets_.global_join(result,pieces.item);
        } else {
            result=sets_.global_join(pieces.before,pieces.item);
            result=sets_.global_join(result,element);
        }
        result=sets_.global_join(result,pieces.after);
        path_set_root_[v]=result;
    }
    path_entry_[t]=element;
    entry_owner_[t]=v;
}
void SleatorTarjanWorstCaseForest::delete_path_set(Node* p,int v) {
    const int t=tail(p);
    if(!path_entry_[t]||entry_owner_[t]!=v)
        throw std::logic_error("dashed path not present in path set");
    auto parts=sets_.split(path_entry_[t]);
    path_set_root_[v]=sets_.global_join(parts.before,parts.after);
    sets_.release_detached_leaf(parts.item);
    path_entry_[t]=nullptr;
    entry_owner_[t]=-1;
}
SleatorTarjanWorstCaseForest::Node*
SleatorTarjanWorstCaseForest::max_path(int v) const {
    Node* r=path_set_root_[v];
    if(!r)return nullptr;
    const int t=r->first->item;
    Node* cached=path_root_[t];
#ifndef NDEBUG
    // Debug validation only: may traverse O(log n), not part of maxwt.
    Node* actual=path(t);
    if(cached!=actual || !cached || !cached->last ||
       cached->last->item!=t || dashed_parent_[t]!=v)
        throw std::logic_error("ST maxwt cached path invalid: owner="+
            std::to_string(v)+" tail="+std::to_string(t)+
            " sameRoot="+std::to_string(cached==actual)+
            " cacheLive="+std::to_string(cached && cached->live)+
            " cacheLast="+std::to_string(cached && cached->last?
                                         cached->last->item:-1)+
            " actualTail="+std::to_string(tail(actual))+
            " dashedOwner="+std::to_string(dashed_parent_[t]));
#endif
    return cached;
}

// ST83 Section 5, published splice procedure, with its path-set changes.
SleatorTarjanWorstCaseForest::Node*
SleatorTarjanWorstCaseForest::splice(Node* p) {
    const int t=tail(p);
    const int v=dashed_parent_[t];
    if(v<0)throw std::logic_error("cannot splice a root path");
    const int dashed_weight=dashed_cost_[t];
    auto parts=split_vertex(v);
    const int old_weight=vertex_[v]->weight;
    const int new_weight=old_weight-weight(p)+weight(parts.before);
    if(new_weight<=0)
        throw std::logic_error("invalid ST leaf weight after splice");
    delete_path_set(p,v);
    paths_.set_detached_leaf_weight(vertex_[v],new_weight);
    dashed_parent_[t]=-1;
    if(parts.before) {
        const int low_tail=tail(parts.before);
        dashed_parent_[low_tail]=v;
        dashed_cost_[low_tail]=parts.before_cost;
        insert_path_set(parts.before,v);
    }
    Node* grown=concatenate(p,vertex_[v],dashed_weight);
    if(parts.after)grown=concatenate(grown,parts.after,parts.after_cost);
    return grown;
}

// ST83 Section 5, published expose procedure, with path-set changes.
SleatorTarjanWorstCaseForest::Node*
SleatorTarjanWorstCaseForest::expose(int u) {
    auto parts=split_vertex(u);
    int revised=vertex_[u]->weight+weight(parts.before);
    paths_.set_detached_leaf_weight(vertex_[u],revised);
    if(parts.before) {
        const int low_tail=tail(parts.before);
        dashed_parent_[low_tail]=u;
        dashed_cost_[low_tail]=parts.before_cost;
        insert_path_set(parts.before,u);
    }
    Node* p=vertex_[u];
    if(parts.after)p=concatenate(p,parts.after,parts.after_cost);
    while(dashed_parent_[tail(p)]>=0)p=splice(p);
    return p;
}

// ST83 Section 5: slice is the inverse of splice, restoring a light
// edge and promoting a formerly dashed heavy child if one exists.
SleatorTarjanWorstCaseForest::Node*
SleatorTarjanWorstCaseForest::slice(Node* p) {
    Node* at=paths_.light(p);
    if(!at)throw std::logic_error("slice requires an existing light edge");
    const int v=at->item;
    auto parts=split_vertex(v);
    if(!parts.before)
        throw std::logic_error("light edge has no lower path");
    paths_.set_detached_leaf_weight(
        vertex_[v],vertex_[v]->weight+weight(parts.before));
    const int lower_tail=tail(parts.before);
    dashed_parent_[lower_tail]=v;
    dashed_cost_[lower_tail]=parts.before_cost;

    Node* heavy=max_path(v);
    Node* s=vertex_[v];
    if(heavy && 2LL*weight(heavy)>vertex_[v]->weight) {
        const int h_tail=tail(heavy);
        const int hcost=dashed_cost_[h_tail];
        delete_path_set(heavy,v);
        paths_.set_detached_leaf_weight(
            vertex_[v],vertex_[v]->weight-weight(heavy));
        dashed_parent_[h_tail]=-1;
        s=concatenate(heavy,vertex_[v],hcost);
    }
    if(parts.after)s=concatenate(s,parts.after,parts.after_cost);
    insert_path_set(parts.before,v);
    (void)s;
    return parts.before;
}

// ST83 Section 5, published conceal procedure.
void SleatorTarjanWorstCaseForest::conceal(Node* p) {
    while(paths_.light(p))p=slice(p);
    const int v=first(p);
    Node* heavy=max_path(v);
    if(heavy && 2LL*weight(heavy)>vertex_[v]->weight) {
        auto parts=split_vertex(v);
        if(parts.before)
            throw std::logic_error("head has a predecessor");
        const int h_tail=tail(heavy),hcost=dashed_cost_[h_tail];
        delete_path_set(heavy,v);
        paths_.set_detached_leaf_weight(
            vertex_[v],vertex_[v]->weight-weight(heavy));
        dashed_parent_[h_tail]=-1;
        Node* s=concatenate(heavy,vertex_[v],hcost);
        if(parts.after)s=concatenate(s,parts.after,parts.after_cost);
        (void)s;
    }
}

void SleatorTarjanWorstCaseForest::evert(int u) {
    verify_vertex(u);
    Node* p=expose(u);
    paths_.reverse(p);
    path_root_[tail(p)]=p;
    dashed_parent_[u]=-1;
    conceal(p);
}
int SleatorTarjanWorstCaseForest::root(int u) {
    verify_vertex(u);
    Node* p=expose(u);
    const int result=tail(p);
    conceal(p);
    return result;
}
void SleatorTarjanWorstCaseForest::link(int u,int v,int w) {
    verify_vertex(u);verify_vertex(v);
    if(u==v || root(u)==root(v))
        throw std::invalid_argument("ST link would form a cycle");
    evert(u);
    Node* p=expose(v);
    Node* united=concatenate(path(u),p,w);
    conceal(united);
}
void SleatorTarjanWorstCaseForest::cut(int u,int v) {
    verify_vertex(u);verify_vertex(v);
    if(u==v)throw std::invalid_argument("invalid self cut");
    evert(u);
    Node* p=expose(v);
    auto parts=split_vertex(v);
    if(parts.before || !parts.after || parts.after->leaf_count!=1 ||
       first(parts.after)!=u)
        throw std::invalid_argument("ST cut requires an existing edge");
    dashed_parent_[v]=-1;
    conceal(vertex_[v]);
    conceal(parts.after);
}
int SleatorTarjanWorstCaseForest::min_on_path(int u,int v) {
    verify_vertex(u);verify_vertex(v);
    if(u==v)return 1000000000;
    if(root(u)!=root(v))
        throw std::invalid_argument("ST path between disconnected nodes");
    evert(u);
    Node* p=expose(v);
    const int result=paths_.min_cost(p);
    conceal(p);
    return result;
}
int SleatorTarjanWorstCaseForest::kth_on_path(int u,int v,int k) {
    verify_vertex(u);verify_vertex(v);
    if(root(u)!=root(v))
        throw std::invalid_argument("ST path between disconnected nodes");
    evert(u);
    Node* p=expose(v);
    const int index=p->leaf_count-1-k;
    const int result=paths_.kth(p,index)->item;
    conceal(p);
    return result;
}
int SleatorTarjanWorstCaseForest::path_length(int u,int v) {
    verify_vertex(u);verify_vertex(v);
    if(root(u)!=root(v))
        throw std::invalid_argument("ST path between disconnected nodes");
    evert(u);
    Node* p=expose(v);
    const int result=p->leaf_count;
    conceal(p);
    return result;
}

std::string SleatorTarjanWorstCaseForest::debug_state() const {
    std::ostringstream out;
    std::unordered_set<Node*> seen;
    for(int v=0;v<static_cast<int>(vertex_.size());++v) {
        Node* p=path(v);
        if(seen.insert(p).second) {
            out<<"solid path: ";
            for(int id:paths_.sequence(p))out<<id<<",";
            out<<" weight="<<p->sum_weight<<"\n";
        }
        out<<"v="<<v<<" wt="<<vertex_[v]->weight
           <<" dashed_parent="<<dashed_parent_[v]
           <<" dashed_cost="<<dashed_cost_[v]
           <<" path_set_owner="<<entry_owner_[v]<<"\n";
    }
    return out.str();
}

void SleatorTarjanWorstCaseForest::check_invariants() {
    std::unordered_set<Node*> visited;
    for(int v=0;v<static_cast<int>(vertex_.size());++v) {
        // ST1983 Section 5: vertex weight equals one plus the
        // weights of all dashed paths entering that vertex.
        // This is a debug-only O(n^2) oracle, never used by operations.
        long long expected=1;
        for(int t=0;t<static_cast<int>(vertex_.size());++t)
            if(dashed_parent_[t]==v)expected+=weight(path(t));
        if(vertex_[v]->weight!=expected)
            throw std::logic_error("ST vertex weight invariant failed at "+
                std::to_string(v)+" expected "+std::to_string(expected)+
                " actual "+std::to_string(vertex_[v]->weight));
        Node* p=path(v);
        if(path_root_[tail(p)]!=p)
            throw std::logic_error("ST path-tail root cache invalid");
        if(visited.insert(p).second)paths_.check_invariants(p);
        if(path_entry_[v]) {
            if(dashed_parent_[v]<0 ||entry_owner_[v]!=dashed_parent_[v] ||
               tail(p)!=v || path_entry_[v]->weight!=weight(p))
                throw std::logic_error("incorrect dashed-path path-set entry");
        } else if(dashed_parent_[v]>=0) {
            throw std::logic_error("dashed edge without path-set entry");
        }
        if(path_set_root_[v]) {
            sets_.check_invariants(path_set_root_[v]);
            auto entries=sets_.sequence(path_set_root_[v]);
            int previous=1000000000;
            for(int t:entries) {
                if(entry_owner_[t]!=v || dashed_parent_[t]!=v)
                    throw std::logic_error("path-set ownership mismatch");
                if(weight(path(t))>previous)
                    throw std::logic_error("path set is not sorted by weight");
                previous=weight(path(t));
            }
        }
    }
}
} // namespace mtp
