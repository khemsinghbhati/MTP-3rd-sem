#include "mtp/globally_biased_binary_tree.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mtp {

int GloballyBiasedBinaryTree::floor_log2(int weight) {
    if(weight<=0)throw std::invalid_argument("nonpositive leaf weight");
    int result=0;
    while(weight>1) { weight/=2; ++result; }
    return result;
}

GloballyBiasedBinaryTree::Node* GloballyBiasedBinaryTree::leaf(
    int item,int weight) {
    if(item<0||weight<=0)throw std::invalid_argument("invalid leaf");
    Node* x;
    if(!leaf_pool_.empty()) {
        x=leaf_pool_.back();leaf_pool_.pop_back();
        *x=Node();
    } else {
        storage_.emplace_back(std::make_unique<Node>());
        x=storage_.back().get();
    }
    x->rank=floor_log2(weight);
    x->item=item;
    x->weight=weight;
    refresh(x);
    return x;
}

void GloballyBiasedBinaryTree::set_detached_leaf_weight(
    Node* x,int new_weight) {
    if(!x||!x->live||!x->is_leaf()||x->parent||new_weight<=0)
        throw std::invalid_argument("reweight requires a detached live leaf");
    x->weight=new_weight;
    x->rank=floor_log2(new_weight);
    refresh(x);
}
void GloballyBiasedBinaryTree::release_detached_leaf(Node* x) {
    if(!x||!x->live||!x->is_leaf()||x->parent)
        throw std::invalid_argument("release requires a detached live leaf");
    *x=Node();
    x->live=false;
    leaf_pool_.push_back(x);
}

GloballyBiasedBinaryTree::Node* GloballyBiasedBinaryTree::internal(
    Node* a,Node* b,int rank,int cost) {
    if(!a||!b)throw std::logic_error("internal node needs both children");
    Node* root;
    if(!pool_.empty()) {root=pool_.back();pool_.pop_back();*root=Node();}
    else {
        storage_.emplace_back(std::make_unique<Node>());
        root=storage_.back().get();
    }
    root->rank=rank;
    root->item=-1;
    root->edge_cost=cost;
    set_left(root,a);
    set_right(root,b);
    refresh(root);
    return root;
}

void GloballyBiasedBinaryTree::refresh(Node* x) {
    constexpr long long infinity=1000000000000LL;
    if(x->is_leaf()) {
        x->sum_weight=x->weight;
        x->left_tilt_min=x->right_tilt_min=infinity;
        x->minimum_cost=1000000000;
        x->first=x->last=x;
        x->leaf_count=1;
        return;
    }
    const Node* a=x->left;
    const Node* b=x->right;
    if(!a||!b)throw std::logic_error("refresh on malformed tree");
    x->sum_weight=a->sum_weight+b->sum_weight;
    x->minimum_cost=std::min({x->edge_cost,a->minimum_cost,b->minimum_cost});
    x->leaf_count=a->leaf_count+b->leaf_count;
    x->first=a->first;
    x->last=b->last;
    x->left_tilt_min=std::min({
        a->left_tilt_min,
        a->sum_weight-static_cast<long long>(b->first->weight),
        b->left_tilt_min==infinity?infinity:
            a->sum_weight+b->left_tilt_min});
    x->right_tilt_min=std::min({
        b->right_tilt_min,
        b->sum_weight-static_cast<long long>(a->last->weight),
        a->right_tilt_min==infinity?infinity:
            b->sum_weight+a->right_tilt_min});
}
void GloballyBiasedBinaryTree::flip(Node* x) {
    if(!x||x->is_leaf())return;
    std::swap(x->left,x->right);
    std::swap(x->first,x->last);
    std::swap(x->left_tilt_min,x->right_tilt_min);
    x->reversed=!x->reversed;
}
void GloballyBiasedBinaryTree::push(Node* x) {
    if(!x||x->is_leaf()||!x->reversed)return;
    flip(x->left);
    flip(x->right);
    x->reversed=false;
}
void GloballyBiasedBinaryTree::reverse(Node* root) {flip(root);}
GloballyBiasedBinaryTree::Node*
GloballyBiasedBinaryTree::kth(Node* root,int position) const {
    if(!root||position<0||position>=root->leaf_count)
        throw std::out_of_range("kth node on path");
    while(!root->is_leaf()) {
        push(root);
        if(position<root->left->leaf_count)root=root->left;
        else {
            position-=root->left->leaf_count;
            root=root->right;
        }
    }
    return root;
}
int GloballyBiasedBinaryTree::min_cost(Node* root) const {
    return root?root->minimum_cost:1000000000;
}
GloballyBiasedBinaryTree::Node*
GloballyBiasedBinaryTree::light(Node* root) const {
    if(!root||root->left_tilt_min>0)return nullptr;
    // Sleator–Tarjan (1983), §5: rightmost violated lefttilt.
    long long offset=0;
    while(!root->is_leaf()) {
        push(root);
        const Node* a=root->left;
        const Node* b=root->right;
        if(b->left_tilt_min!=1000000000000LL &&
           offset+a->sum_weight+b->left_tilt_min<=0) {
            offset+=a->sum_weight;
            root=root->right;
        } else if(offset+a->sum_weight-b->first->weight<=0) {
            return b->first;
        } else if(a->left_tilt_min!=1000000000000LL &&
                  offset+a->left_tilt_min<=0) {
            root=root->left;
        } else return nullptr;
    }
    return nullptr;
}

// Bent, Sleator & Tarjan (1985), "Biased Search Trees", Section 4,
// paragraph after Figure 14, Figure 15 and Theorem 12.
// This is the paper's BOTTOM-UP builder, not k global_join calls.
//
// Each internal binary-tree node represents the separator/boundary
// between consecutive leaves (Sleator-Tarjan 1983 Section 5). The
// boundary cost stays with the internal node under tilt-left rotations.
// No path aggregate is consulted while constructing; we compute
// all aggregates by ONE postorder traversal after the rank topology
// is complete. Thus maintaining aggregates does not spoil O(k).
GloballyBiasedBinaryTree::Node*
GloballyBiasedBinaryTree::build_linear(
    const std::vector<Node*>& leaves,
    const std::vector<int>& boundary_cost) {
    if(leaves.empty()) {
        if(!boundary_cost.empty())
            throw std::invalid_argument("empty leaf sequence has costs");
        return nullptr;
    }
    if(boundary_cost.size()+1!=leaves.size())
        throw std::invalid_argument("one cost per leaf boundary required");
    linear_steps_=0;
    for(Node* l:leaves)
        if(!l||!l->is_leaf()||l->parent||!l->live)
            throw std::invalid_argument("linear builder requires detached live leaves");
    Node* root=leaves.front();
    Node* rightmost=root;
    for(std::size_t i=1;i<leaves.size();++i) {
        Node* y=leaves[i];
        Node* x=rightmost;
        // Figure 15: find the maximal right-path node whose
        // parent's rank does not exceed rank(y).
        while(x->parent && x->parent->rank<=y->rank) {
            x=x->parent;
            ++linear_steps_;
        }
        Node* old_parent=x->parent;
        Node* added=internal(x,y,std::max(x->rank,y->rank)+1,
                             boundary_cost[i-1]);
        ++linear_steps_;
        if(!old_parent) {
            root=added;
        } else {
            // x was on the rightmost spine of the old search tree.
            if(old_parent->right!=x)
                throw std::logic_error("linear builder lost rightmost spine");
            set_right(old_parent,added);
            Node* current=old_parent;
            for(;;) {
                Node* ancestor=current->parent;
                if(ancestor && ancestor->right!=current)
                    throw std::logic_error("linear tilt left spine broken");
                const int previous_rank=current->rank;
                Node* revised=tilt_left(current);
                if(ancestor)set_right(ancestor,revised);
                else root=revised;
                ++linear_steps_;
                // The paper stops after the first tilt without a rank
                // promotion. Rotations preserve the rank of that parent.
                if(revised!=current || current->rank==previous_rank ||
                   !ancestor)
                    break;
                current=ancestor;
            }
        }
        rightmost=y;
    }

    // Recompute subtree sums, path minima, first/last, and tilt minima
    // once in O(k), without adding O(log k) per inserted leaf.
    std::vector<std::pair<Node*,bool>> stack{{root,false}};
    while(!stack.empty()) {
        const auto [x,done]=stack.back();stack.pop_back();
        if(!done) {
            stack.push_back({x,true});
            if(!x->is_leaf()) {
                stack.push_back({x->right,false});
                stack.push_back({x->left,false});
            }
        } else {
            refresh(x);
            ++linear_steps_;
        }
    }
    root->parent=nullptr;
    return root;
}

void GloballyBiasedBinaryTree::set_left(Node* p,Node* c) {
    if(!p||!c)throw std::logic_error("invalid left pointer");
    p->left=c;c->parent=p;
}
void GloballyBiasedBinaryTree::set_right(Node* p,Node* c) {
    if(!p||!c)throw std::logic_error("invalid right pointer");
    p->right=c;c->parent=p;
}
void GloballyBiasedBinaryTree::recycle(Node* x) {
    if(!x||x->is_leaf())throw std::logic_error("recycling a leaf");
    *x=Node();x->live=false;pool_.push_back(x);
}

// Bent–Sleator–Tarjan (1985), Section 4, "tilt left".
GloballyBiasedBinaryTree::Node*
GloballyBiasedBinaryTree::tilt_left(Node* x) {
    push(x);
    if(x->is_leaf())return x;
    if(x->left->rank==x->rank && x->right->rank==x->rank) {
        ++x->rank;refresh(x);return x;
    }
    if(x->right->rank==x->rank && x->left->rank!=x->rank) {
        Node* y=x->right;
        push(y);
        Node* beta=y->left;
        set_right(x,beta);
        set_left(y,x);
        y->parent=nullptr;
        refresh(x);refresh(y);
        return y;
    }
    return x;
}
GloballyBiasedBinaryTree::Node*
GloballyBiasedBinaryTree::tilt_right(Node* x) {
    push(x);
    if(x->is_leaf())return x;
    if(x->left->rank==x->rank && x->right->rank==x->rank) {
        ++x->rank;return x;
    }
    if(x->left->rank==x->rank && x->right->rank!=x->rank) {
        Node* y=x->left;
        push(y);
        Node* beta=y->right;
        set_left(x,beta);
        set_right(y,x);
        y->parent=nullptr;
        refresh(x);refresh(y);
        return y;
    }
    return x;
}

// The locally biased join is used ONLY in split (B–S–T 1985 §4).
GloballyBiasedBinaryTree::Node*
GloballyBiasedBinaryTree::local_join(Node* x,Node* y,int edge_cost) {
    push(x);push(y);
    if(!x)return y;
    if(!y)return x;
    if(x->rank==y->rank ||
       (x->rank>y->rank&&x->is_leaf()) ||
       (x->rank<y->rank&&y->is_leaf())) {
        return internal(x,y,std::max(x->rank,y->rank)+1,edge_cost);
    }
    if(x->rank>y->rank) {
        x=tilt_left(x);
        Node* z=x->right;
        z->parent=nullptr;
        Node* joined=local_join(z,y,edge_cost);
        set_right(x,joined);
        refresh(x);
        x->parent=nullptr;
        return x;
    }
    y=tilt_right(y);
    Node* z=y->left;
    z->parent=nullptr;
    Node* joined=local_join(x,z,edge_cost);
    set_left(y,joined);
    refresh(y);
    y->parent=nullptr;
    return y;
}

// Global join cases 1–4b(v): Bent–Sleator–Tarjan (1985), §4,
// Figure 14, including all equal-rank cases.
GloballyBiasedBinaryTree::Node*
GloballyBiasedBinaryTree::global_join(Node* x,Node* y,int edge_cost) {
    push(x);push(y);
    if(!x)return y;
    if(!y)return x;
    if(x->rank==y->rank) {
        if(x->is_leaf()||y->is_leaf())
            return internal(x,y,x->rank+1,edge_cost);
        // Equal rank, both internal: Case 4.
        const int s=x->rank;
        Node* u=x->right->rank<s ? x : x->right;
        Node* v=y->left->rank<s ? y : y->left;
        // Case 4 inspects grandchildren. Propagate pending path
        // reversals of these child nodes before selecting their arcs.
        push(u);
        push(v);
        Node* ux=u->right;
        Node* vy=v->left;
        ux->parent=nullptr;
        vy->parent=nullptr;
        Node* z=global_join(ux,vy,edge_cost);
        if(z->rank==s) { // Case 4a
            Node* zl=z->left;
            Node* zr=z->right;
            set_right(u,zl);
            set_left(v,zr);
            set_left(z,x);
            set_right(z,y);
            z->rank=s+1;
            if(u!=x)refresh(u);
            if(v!=y)refresh(v);
            refresh(x);refresh(y);refresh(z);
            z->parent=nullptr;
            return z;
        }
        if(z->rank>s)
            throw std::logic_error("global join exceeded common rank");
        if(u==x->right) { // Case 4b(i)
            Node* ul=u->left;
            set_right(x,ul);
            set_left(v,z);
            set_left(u,x);
            set_right(u,y);
            u->rank=s+1;
            refresh(x);refresh(v);
            if(v!=y)refresh(y);
            refresh(u);
            u->parent=nullptr;
            return u;
        }
        if(v==y->left) { // Case 4b(ii)
            Node* vr=v->right;
            set_left(y,vr);
            set_right(u,z);
            set_left(v,x);
            set_right(v,y);
            v->rank=s+1;
            refresh(u);if(u!=x)refresh(x);
            refresh(y);refresh(v);
            v->parent=nullptr;
            return v;
        }
        if(u==x && x->left->rank==s) { // Case 4b(iii)
            set_left(v,z);
            set_right(x,y);
            ++x->rank;
            refresh(v);if(v!=y)refresh(y);
            refresh(x);
            x->parent=nullptr;
            return x;
        }
        if(v==y && y->right->rank==s) { // Case 4b(iv)
            set_right(u,z);
            set_left(y,x);
            ++y->rank;
            refresh(u);if(u!=x)refresh(x);
            refresh(y);
            y->parent=nullptr;
            return y;
        }
        if(u==x&&v==y&&x->left->rank<s&&y->right->rank<s) {
            // Case 4b(v).
            set_left(v,z);
            set_right(x,y);
            refresh(v);refresh(y);refresh(x);
            x->parent=nullptr;
            return x;
        }
        throw std::logic_error("no Bent–Sleator–Tarjan global-join case");
    }
    // Cases 1–3 (unequal rank).
    if(x->rank>y->rank) {
        if(x->is_leaf())return internal(x,y,x->rank+1,edge_cost);
        x=tilt_left(x);
        Node* old_right=x->right;old_right->parent=nullptr;
        set_right(x,global_join(old_right,y,edge_cost));
        refresh(x);
        x->parent=nullptr;
        return x;
    }
    if(y->is_leaf())return internal(x,y,y->rank+1,edge_cost);
    y=tilt_right(y);
    Node* old_left=y->left;old_left->parent=nullptr;
    set_left(y,global_join(x,old_left,edge_cost));
    refresh(y);
    y->parent=nullptr;
    return y;
}

GloballyBiasedBinaryTree::Split GloballyBiasedBinaryTree::split(Node* x) {
    if(!x||!x->is_leaf()||!x->live)
        throw std::invalid_argument("split requires active leaf");
    Node* left=nullptr;
    Node* right=nullptr;
    // Push pending reverse flags from the root down to the selected leaf.
    std::vector<Node*> ancestry;
    for(Node* u=x;u;u=u->parent)ancestry.push_back(u);
    for(auto it=ancestry.rbegin();it!=ancestry.rend();++it)push(*it);
    Node* current=x;
    std::vector<Node*> detached_internals;
    int before_cost=0,after_cost=0;
    while(current->parent) {
        Node* p=current->parent;
        Node* up=p->parent;
        if(p->left==current) {
            Node* sibling=p->right;
            sibling->parent=nullptr;
            if(!right)after_cost=p->edge_cost;
            right=right?local_join(right,sibling,p->edge_cost):sibling;
        } else if(p->right==current) {
            Node* sibling=p->left;
            sibling->parent=nullptr;
            if(!left)before_cost=p->edge_cost;
            left=left?local_join(sibling,left,p->edge_cost):sibling;
        } else {
            throw std::logic_error("malformed parent while splitting");
        }
        current=p;
        current->parent=up;
        // Preserve the ancestral chain until after the entire split.
        // Recycling a parent here erases its parent pointer and would
        // silently truncate a multi-level split.
        detached_internals.push_back(p);
    }
    for(Node* destroyed:detached_internals)recycle(destroyed);
    x->parent=nullptr;
    if(left)left->parent=nullptr;
    if(right)right->parent=nullptr;
    return {left,x,right,before_cost,after_cost};
}

std::vector<int> GloballyBiasedBinaryTree::sequence(Node* root) const {
    std::vector<int> answer;
    if(!root)return answer;
    std::vector<Node*> stack{root};
    while(!stack.empty()) {
        Node* u=stack.back();stack.pop_back();
        push(u); // realize a pending path reversal before visiting children
        if(u->is_leaf())answer.push_back(u->item);
        else {
            if(!u->left||!u->right)
                throw std::logic_error("not a full binary tree");
            stack.push_back(u->right);
            stack.push_back(u->left);
        }
    }
    return answer;
}

void GloballyBiasedBinaryTree::check_invariants(Node* root) const {
    if(!root)return;
    if(root->parent)
        throw std::logic_error("represented tree has a parent");
    std::vector<Node*> stack{root};
    std::unordered_set<Node*> visited;
    std::unordered_map<Node*,std::pair<int,int>> span;
    std::vector<Node*> leaves;
    while(!stack.empty()) {
        auto* u=stack.back();stack.pop_back();
        push(u);
        if(!u||!u->live||!visited.insert(u).second)
            throw std::logic_error("inactive or repeated node");
        // Bent–Sleator–Tarjan 1985 §4: rank(root) is bounded
        // by the logarithm of the total external weight.
        if(u->sum_weight<1 || u->sum_weight>1000000000LL ||
           u->rank>1+floor_log2(static_cast<int>(u->sum_weight)))
            throw std::logic_error("global biased rank/weight upper bound");
        if(u->is_leaf()) {
            if(u->left||u->right || u->rank!=floor_log2(u->weight))
                throw std::logic_error("leaf rank invariant");
            leaves.push_back(u);
            continue;
        }
        if(!u->left||!u->right)
            throw std::logic_error("internal node needs two children");
        for(Node* child:{u->left,u->right}) {
            if(child->parent!=u)
                throw std::logic_error("parent pointer incorrect");
            if(child->rank>u->rank ||
               (child->is_leaf()&&child->rank==u->rank))
                throw std::logic_error("rank monotonicity");
            if(!child->is_leaf()) {
                for(Node* grand:{child->left,child->right})
                    if(grand && grand->rank>=u->rank)
                        throw std::logic_error("grandchild rank invariant");
            }
        }
        stack.push_back(u->right);stack.push_back(u->left);
    }
    // Global bias: a neighboring leaf of a minor child must have
    // rank at least parent.rank-1 (Bent–Sleator–Tarjan §4(iv')).
    std::vector<Node*> inorder;
    std::vector<Node*> walk;
    Node* cur=root;
    while(cur || !walk.empty()) {
        if(cur) {
            walk.push_back(cur);
            cur=cur->is_leaf()?nullptr:cur->left;
        } else {
            Node* x=walk.back();walk.pop_back();
            if(x->is_leaf())inorder.push_back(x);
            cur=x->is_leaf()?nullptr:x->right;
        }
    }
    std::unordered_map<Node*,std::pair<int,int>> interval;
    std::vector<std::pair<Node*,bool>> todo{{root,false}};
    int leaf_number=0;
    while(!todo.empty()) {
        auto [u,exit]=todo.back();todo.pop_back();
        if(!exit) {
            todo.push_back({u,true});
            if(!u->is_leaf()) {
                todo.push_back({u->right,false});
                todo.push_back({u->left,false});
            }
        } else if(u->is_leaf()) {
            interval[u]={leaf_number,leaf_number};
            ++leaf_number;
        } else {
            interval[u]={interval[u->left].first,
                         interval[u->right].second};
        }
    }
    if(leaf_number!=static_cast<int>(inorder.size()))
        throw std::logic_error("inorder length mismatch");
    for(auto* p:visited) {
        if(p->is_leaf())continue;
        for(Node* child:{p->left,p->right}) {
            if(p->rank-child->rank<2)continue;
            const auto [a,b]=interval.at(child);
            if(a>0 && inorder[a-1]->rank<p->rank-1)
                throw std::logic_error("global bias left neighbor");
            if(b+1<leaf_number && inorder[b+1]->rank<p->rank-1)
                throw std::logic_error("global bias right neighbor");
        }
    }
}
} // namespace mtp
