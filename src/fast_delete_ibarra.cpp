#include "mtp/fast_delete_ibarra.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace mtp {
int FastCliqueIntersectionIndex::bound(int edges) {
    return std::max(1,static_cast<int>(
        std::ceil(std::sqrt(static_cast<double>(edges)))));
}
FastCliqueIntersectionIndex::FastCliqueIntersectionIndex(int n,int m)
    : n_(n),threshold_(bound(m)),elements_(n+2),
      common_(n,std::vector<int>(n,0)) {
    if(n<0||m<0)throw std::invalid_argument("negative graph size");
}
void FastCliqueIntersectionIndex::add_member(int id,int v) {
    auto& e=elements_[id];
    if(e.member_bit[v]||e.overflow_bit[v])
        throw std::logic_error("duplicate clique member");
    for(int w:e.member){++common_[v][w];++common_[w][v];}
    ++common_[v][v];
    e.member.push_back(v);
    e.member_iterator[v]=std::prev(e.member.end());
    e.member_bit[v]=1;
}
void FastCliqueIntersectionIndex::remove_member(int id,int v) {
    auto& e=elements_[id];
    if(!e.member_bit[v])throw std::logic_error("unknown clique member");
    for(int w:e.member) {
        if(w==v)continue;
        --common_[v][w];--common_[w][v];
    }
    --common_[v][v];
    e.member.erase(e.member_iterator[v]);
    e.member_bit[v]=0;
}
void FastCliqueIntersectionIndex::add_overflow(int id,int v) {
    auto& e=elements_[id];
    if(e.member_bit[v]||e.overflow_bit[v])
        throw std::logic_error("duplicate overflow member");
    if(e.overflow.empty()) {
        overflows_.push_back(id);
        e.overflow_global_iterator=std::prev(overflows_.end());
        e.in_overflows=true;
    }
    e.overflow.push_back(v);
    e.overflow_iterator[v]=std::prev(e.overflow.end());
    e.overflow_bit[v]=1;
}
void FastCliqueIntersectionIndex::remove_overflow(int id,int v) {
    auto& e=elements_[id];
    if(!e.overflow_bit[v])throw std::logic_error("unknown overflow member");
    e.overflow.erase(e.overflow_iterator[v]);
    e.overflow_bit[v]=0;
    if(e.overflow.empty()) {
        overflows_.erase(e.overflow_global_iterator);
        e.in_overflows=false;
    }
}
void FastCliqueIntersectionIndex::add_clique(
        int id,const std::vector<unsigned char>& bits) {
    if(id<0||id>=static_cast<int>(elements_.size())||
       static_cast<int>(bits.size())!=n_||elements_[id].active)
        throw std::invalid_argument("invalid clique addition");
    auto& e=elements_[id];
    e.active=true;
    e.member_bit.assign(n_,0);e.overflow_bit.assign(n_,0);
    e.member_iterator.resize(n_);e.overflow_iterator.resize(n_);
    for(int v=0;v<n_;++v) {
        if(!bits[v])continue;
        if(static_cast<int>(e.member.size())<threshold_)add_member(id,v);
        else add_overflow(id,v);
    }
}
void FastCliqueIntersectionIndex::remove_clique(int id) {
    if(id<0||id>=static_cast<int>(elements_.size())||
       !elements_[id].active)
        throw std::invalid_argument("invalid clique removal");
    auto& e=elements_[id];
    while(!e.member.empty())remove_member(id,e.member.front());
    while(!e.overflow.empty())remove_overflow(id,e.overflow.front());
    e.active=false;
}
void FastCliqueIntersectionIndex::rebalance(int m) {
    threshold_=bound(m);
    for(int id=0;id<static_cast<int>(elements_.size());++id) {
        auto& e=elements_[id];
        if(!e.active)continue;
        while(static_cast<int>(e.member.size())>threshold_) {
            int v=e.member.back();
            remove_member(id,v);add_overflow(id,v);
        }
        while(!e.overflow.empty()&&
              static_cast<int>(e.member.size())<threshold_) {
            int v=e.overflow.front();
            remove_overflow(id,v);add_member(id,v);
        }
    }
}
int FastCliqueIntersectionIndex::count_common_cliques(int u,int v) const {
    if(u<0||v<0||u>=n_||v>=n_)
        throw std::out_of_range("invalid vertex");
    int count=common_[u][v];
    // Ibarra Section 6.2: only overflow elements may contribute
    // intersections not already included in COMMON[u,v].
    for(int id:overflows_) {
        const auto& e=elements_[id];
        if((e.member_bit[u]||e.overflow_bit[u])&&
           (e.member_bit[v]||e.overflow_bit[v])&&
           (e.overflow_bit[u]||e.overflow_bit[v]))++count;
    }
    return count;
}
} // namespace mtp
