#include "mtp/dynamic_split_ibarra.hpp"
#include <iterator>
#include <stdexcept>
#include <utility>

namespace mtp {
DynamicSplitGraph::DynamicSplitGraph(const Graph& graph) {
    const int n = graph.vertex_count();
    degree_.resize(n);
    std::vector<int> counts(n+1,0);
    for (int v=0;v<n;++v) {
        degree_[v]=static_cast<int>(graph.neighbors(v).size());
        ++counts[degree_[v]];
    }
    first_.resize(n+1);
    last_.resize(n+1);
    present_.resize(n+1,false);
    int index=0;
    for (int d=n;d>=0;--d) {
        for (int k=0;k<counts[d];++k) {
            auto it=sequence_.insert(sequence_.end(),Entry{d,false});
            if(!present_[d]) {first_[d]=it;present_[d]=true;}
            last_[d]=it;
            ++index;
            if(d>=index-1) {j_=index;boundary_=it;}
        }
    }
    index=0;
    for(auto it=sequence_.begin();it!=sequence_.end();++it) {
        it->left=(index<j_);
        if(it->left)sum_left_+=it->degree;
        else sum_right_+=it->degree;
        ++index;
    }
}
bool DynamicSplitGraph::is_split() const noexcept {
    // Hammer and Simeone (1981), used by Ibarra (2008), Theorem 8.
    return sum_left_==static_cast<long long>(j_)*(j_-1)+sum_right_;
}
int DynamicSplitGraph::degree(int vertex) const {
    if(vertex<0||vertex>=static_cast<int>(degree_.size()))
        throw std::out_of_range("invalid vertex");
    return degree_[vertex];
}
void DynamicSplitGraph::validate(int u,int v) const {
    if(u==v||u<0||v<0||
       u>=static_cast<int>(degree_.size())||
       v>=static_cast<int>(degree_.size()))
        throw std::invalid_argument("invalid edge endpoints");
}
void DynamicSplitGraph::increase(Iterator it) {
    const int d=it->degree;
    if(d+1>=static_cast<int>(present_.size()))
        throw std::invalid_argument("invalid duplicate insertion");
    const auto next=std::next(it);
    if(next!=sequence_.end()&&next->degree==d) first_[d]=next;
    else present_[d]=false;
    if(!present_[d+1]) {present_[d+1]=true;first_[d+1]=it;}
    last_[d+1]=it;
    ++it->degree;
    if(it->left)++sum_left_;else ++sum_right_;
}
void DynamicSplitGraph::decrease(Iterator it) {
    const int d=it->degree;
    if(d==0)throw std::invalid_argument("invalid deletion: zero degree");
    if(it!=sequence_.begin()&&std::prev(it)->degree==d)
        last_[d]=std::prev(it);
    else present_[d]=false;
    if(!present_[d-1]) {present_[d-1]=true;last_[d-1]=it;}
    first_[d-1]=it;
    --it->degree;
    if(it->left)--sum_left_;else --sum_right_;
}
void DynamicSplitGraph::update_boundary_after_insert() {
    if(j_==static_cast<int>(degree_.size()))return;
    const auto next=std::next(boundary_);
    if(next!=sequence_.end()&&next->degree>=j_) {
        next->left=true;
        sum_left_+=next->degree;sum_right_-=next->degree;
        boundary_=next;++j_;
    }
}
void DynamicSplitGraph::update_boundary_after_delete() {
    if(j_<=1||boundary_->degree>=j_-1)return;
    auto old=boundary_;
    boundary_=std::prev(boundary_);
    old->left=false;
    sum_left_-=old->degree;sum_right_+=old->degree;
    --j_;
}
void DynamicSplitGraph::insert_edge(int u,int v) {
    validate(u,v);
    const int du=degree_[u],dv=degree_[v];
    // Ibarra Section 8: choose first one/two elements of degree classes.
    // Incrementing those entries preserves descending degree order.
    if(du==dv) {
        auto first=first_[du];
        const auto second=std::next(first);
        if(second==sequence_.end()||second->degree!=du)
            throw std::logic_error("invalid degree multiplicity");
        increase(first);increase(second);
    } else if(du>dv) {
        increase(first_[du]);increase(first_[dv]);
    } else {
        increase(first_[dv]);increase(first_[du]);
    }
    ++degree_[u];++degree_[v];
    update_boundary_after_insert();
}
void DynamicSplitGraph::delete_edge(int u,int v) {
    validate(u,v);
    const int du=degree_[u],dv=degree_[v];
    // Symmetric degree-list update using last elements of degree classes.
    if(du==dv) {
        auto last=last_[du];
        const auto previous=std::prev(last);
        if(previous->degree!=du)
            throw std::logic_error("invalid degree multiplicity");
        decrease(last);decrease(previous);
    } else if(du<dv) {
        decrease(last_[du]);decrease(last_[dv]);
    } else {
        decrease(last_[dv]);decrease(last_[du]);
    }
    --degree_[u];--degree_[v];
    update_boundary_after_delete();
}
} // namespace mtp
