#include "mtp/worst_case_dynamic_tree.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace mtp {

SleatorTarjanWorstCaseForest::SleatorTarjanWorstCaseForest(int n)
    : vertex_(n), path_set_root_(n,nullptr), path_entry_(n,nullptr),
      dashed_parent_(n,-1), dashed_cost_(n,0), entry_owner_(n,-1) {
    if(n<0)throw std::invalid_argument("negative node count");
    for(int v=0;v<n;++v)vertex_[v]=paths_.leaf(v,1);
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
SleatorTarjanWorstCaseForest::Node*
SleatorTarjanWorstCaseForest::concatenate(Node* p,Node* q,int c) {
    return paths_.global_join(p,q,c);
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
    return r?path(r->first->item):nullptr;
}

// ST83 Section 5, published splice procedure, with its path-set changes.
SleatorTarjanWorstCaseForest::Node*
SleatorTarjanWorstCaseForest::splice(Node* p) {
    const int t=tail(p);
    const int v=dashed_parent_[t];
    if(v<0)throw std::logic_error("cannot splice a root path");
    const int dashed_weight=dashed_cost_[t];
    auto parts=paths_.split(vertex_[v]);
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
    auto parts=paths_.split(vertex_[u]);
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
    auto parts=paths_.split(vertex_[v]);
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
        auto parts=paths_.split(vertex_[v]);
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
    auto parts=paths_.split(vertex_[v]);
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
