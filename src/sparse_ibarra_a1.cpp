#include "mtp/sparse_ibarra_a1.hpp"
#include "mtp/clique_tree_blair_peyton.hpp"
#include "mtp/mcs.hpp"

#include <algorithm>
#include <queue>
#include <stdexcept>
#include <utility>

namespace mtp {

SparseIbarraChordalGraphA1::SparseIbarraChordalGraphA1(Graph graph)
    : graph_(std::move(graph)), nodes_of_(graph_.vertex_count()) {
    const auto peo=maximum_cardinality_search(graph_);
    if(!is_perfect_elimination_ordering(graph_,peo))
        throw std::invalid_argument("initial A1 graph must be chordal");
    const auto tree=build_clique_tree_blair_peyton(graph_,peo);
    nodes_.resize(static_cast<std::size_t>(graph_.vertex_count()+2));
    for(const auto& clique:tree.cliques)allocate_node(clique);
    for(const auto& edge:tree.edges)link(edge.first,edge.second,edge.weight);
}

void SparseIbarraChordalGraphA1::check_vertices(int u,int v) const {
    const int n=graph_.vertex_count();
    if(u<0||v<0||u>=n||v>=n)
        throw std::out_of_range("A1 vertex outside graph");
}

void SparseIbarraChordalGraphA1::replace_members(
        int id,std::vector<int> vertices) {
    auto& node=nodes_.at(id);
    if(node.members.size()!=node.inverse_iterators.size())
        throw std::logic_error("A1 incidence handles out of sync");
    for(std::size_t i=0;i<node.members.size();++i)
        nodes_of_[node.members[i]].erase(node.inverse_iterators[i]);
    membership_count_-=node.members.size();
    node.members=std::move(vertices);
    node.inverse_iterators.clear();
    node.inverse_iterators.reserve(node.members.size());
    for(int vertex:node.members) {
        if(vertex<0||vertex>=graph_.vertex_count())
            throw std::logic_error("invalid clique member");
        nodes_of_[vertex].push_back(id);
        node.inverse_iterators.push_back(std::prev(nodes_of_[vertex].end()));
    }
    membership_count_+=node.members.size();
}

int SparseIbarraChordalGraphA1::allocate_node(std::vector<int> vertices) {
    int id;
    if(!free_nodes_.empty()) {
        id=free_nodes_.back();
        free_nodes_.pop_back();
    } else {
        id=active_count_;
        while(id<static_cast<int>(nodes_.size())&&nodes_[id].active)++id;
    }
    if(id>=static_cast<int>(nodes_.size()))
        throw std::logic_error("A1 clique capacity exceeded");
    if(nodes_[id].active || !nodes_[id].incident.empty())
        throw std::logic_error("A1 recycled clique not detached");
    nodes_[id].active=true;
    replace_members(id,std::move(vertices));
    ++active_count_;
    return id;
}

void SparseIbarraChordalGraphA1::retire_node(int id) {
    auto& node=nodes_.at(id);
    if(!node.active||!node.incident.empty())
        throw std::logic_error("A1 retiring linked/inactive clique");
    replace_members(id,{});
    node.active=false;
    free_nodes_.push_back(id);
    --active_count_;
}

int SparseIbarraChordalGraphA1::link(int a,int b,int weight) {
    if(a==b || !nodes_[a].active || !nodes_[b].active)
        throw std::logic_error("A1 invalid clique-tree link");
    int id;
    if(!free_edges_.empty()) {
        id=free_edges_.back();
        free_edges_.pop_back();
    } else {
        id=static_cast<int>(edges_.size());
        edges_.emplace_back();
    }
    nodes_[a].incident.push_back(id);
    const auto it_a=std::prev(nodes_[a].incident.end());
    nodes_[b].incident.push_back(id);
    const auto it_b=std::prev(nodes_[b].incident.end());
    auto& edge=edges_[id];
    edge.active=true;
    edge.a=a; edge.b=b; edge.weight=weight;
    edge.in_a=it_a; edge.in_b=it_b;
    return id;
}

void SparseIbarraChordalGraphA1::cut(int id) {
    auto& edge=edges_.at(id);
    if(!edge.active)throw std::logic_error("A1 cutting inactive tree edge");
    nodes_[edge.a].incident.erase(edge.in_a);
    nodes_[edge.b].incident.erase(edge.in_b);
    edge.active=false;
    free_edges_.push_back(id);
}

void SparseIbarraChordalGraphA1::contract(int keep,int drop) {
    if(keep==drop)throw std::logic_error("A1 self contraction");
    std::vector<std::pair<int,int>> neighbors;
    for(int edge_id:nodes_[drop].incident) {
        const auto& e=edges_[edge_id];
        neighbors.push_back({e.a==drop?e.b:e.a,edge_id});
    }
    for(const auto& [other,id]:neighbors) {
        const int weight=edges_[id].weight;
        cut(id);
        if(other!=keep)link(keep,other,weight);
    }
    retire_node(drop);
}

int SparseIbarraChordalGraphA1::clique_of_edge(int u,int v) const {
    // Inverse incidence lists allow a batched, deterministic O(n) query.
    std::vector<unsigned char> marked(nodes_.size(),0);
    for(int id:nodes_of_[u])marked[id]=1;
    int found=-1;
    for(int id:nodes_of_[v])if(marked[id]) {
        if(found!=-1)return -1;
        found=id;
    }
    return found;
}

int SparseIbarraChordalGraphA1::overlap(int a,int b) const {
    // One O(n) vertex-mark array, never a c-by-n characteristic matrix.
    std::vector<unsigned char> marked(graph_.vertex_count(),0);
    for(int v:nodes_[a].members)marked[v]=1;
    int count=0;
    for(int v:nodes_[b].members)count+=marked[v];
    return count;
}

SparseIbarraChordalGraphA1::EndpointPair
SparseIbarraChordalGraphA1::closest_pair(int u,int v) const {
    // Multi-source BFS from the clique subtree T_u to T_v.
    // The first reached T_v node and its T_u ancestor are closest.
    EndpointPair answer;
    const int cap=static_cast<int>(nodes_.size());
    std::vector<unsigned char> target(cap,0);
    std::vector<int> previous(cap,-1),via(cap,-1);
    for(int id:nodes_of_[v])target[id]=1;
    std::queue<int> q;
    for(int id:nodes_of_[u]) {
        q.push(id);
        previous[id]=id;
    }
    if(q.empty())throw std::logic_error("A1 source vertex missing");
    while(!q.empty()) {
        const int current=q.front();q.pop();
        if(target[current]) {
            answer.y=current;
            break;
        }
        for(int id:nodes_[current].incident) {
            const auto& edge=edges_[id];
            const int next=edge.a==current?edge.b:edge.a;
            if(previous[next]>=0)continue;
            previous[next]=current;
            via[next]=id;
            q.push(next);
        }
    }
    if(answer.y<0)throw std::logic_error("A1 disconnected clique tree");
    int current=answer.y;
    while(previous[current]!=current) {
        answer.path_edges.push_back(via[current]);
        current=previous[current];
    }
    answer.x=current;
    return answer;
}

bool SparseIbarraChordalGraphA1::delete_query(int u,int v) const {
    check_vertices(u,v);
    if(u==v)return false;
    if(fast_)return fast_->count_common_cliques(u,v)==1;
    if(!graph_.has_edge(u,v))return false;
    return clique_of_edge(u,v)>=0;
}

bool SparseIbarraChordalGraphA1::insert_query(int u,int v) const {
    check_vertices(u,v);
    if(u==v||graph_.has_edge(u,v))return false;
    const auto pair=closest_pair(u,v);
    if(pair.path_edges.empty())return false;
    const int common=overlap(pair.x,pair.y);
    int minimum=graph_.vertex_count();
    for(int id:pair.path_edges)
        minimum=std::min(minimum,edges_[id].weight);
    return minimum==common;
}

void SparseIbarraChordalGraphA1::add_to_fast_index(int id) {
    std::vector<unsigned char> bits(graph_.vertex_count(),0);
    for(int v:nodes_[id].members)bits[v]=1;
    fast_->add_clique(id,bits);
}

void SparseIbarraChordalGraphA1::enable_fast_delete_queries() {
    if(fast_)return;
    fast_=std::make_unique<FastCliqueIntersectionIndex>(
        graph_.vertex_count(),static_cast<int>(graph_.edge_count()));
    for(int id=0;id<static_cast<int>(nodes_.size());++id)
        if(nodes_[id].active)add_to_fast_index(id);
}

bool SparseIbarraChordalGraphA1::delete_edge(int u,int v) {
    if(!delete_query(u,v))return false;
    const int x=clique_of_edge(u,v);
    if(x<0)throw std::logic_error("A1 unique deletion clique absent");
    const int k=static_cast<int>(nodes_[x].members.size());
    if(fast_)fast_->remove_clique(x);

    std::vector<int> xu,xv;
    xu.reserve(k);xv.reserve(k);
    for(int w:nodes_[x].members) {
        if(w!=v)xu.push_back(w);
        if(w!=u)xv.push_back(w);
    }

    // O(n) batch node membership tests, including high-degree clique stars.
    std::vector<unsigned char> mark(nodes_.size(),0);
    for(int id:nodes_of_[u])mark[id]|=1;
    for(int id:nodes_of_[v])mark[id]|=2;
    struct Neighbor {int other,weight,side;};
    std::vector<Neighbor> neighbors;
    int absorb_u=-1,absorb_v=-1;
    for(int id:nodes_[x].incident) {
        const auto& e=edges_[id];
        const int other=e.a==x?e.b:e.a;
        const int side=(mark[other]&1)?0:((mark[other]&2)?1:2);
        neighbors.push_back({other,e.weight,side});
        if(e.weight==k-1 && side==0)absorb_u=other;
        if(e.weight==k-1 && side==1)absorb_v=other;
    }

    const std::vector<int> old_edges(
        nodes_[x].incident.begin(),nodes_[x].incident.end());
    for(int id:old_edges)cut(id);
    replace_members(x,std::move(xu));
    const int x2=allocate_node(std::move(xv));
    for(const auto& neighbor:neighbors)
        link(neighbor.side==1?x2:x,neighbor.other,neighbor.weight);
    link(x,x2,k-2);
    if(absorb_u>=0)contract(absorb_u,x);
    if(absorb_v>=0)contract(absorb_v,x2);
    graph_.remove_edge(u,v);
    if(fast_) {
        if(nodes_[x].active)add_to_fast_index(x);
        if(nodes_[x2].active)add_to_fast_index(x2);
        fast_->rebalance(static_cast<int>(graph_.edge_count()));
    }
    return true;
}

bool SparseIbarraChordalGraphA1::insert_edge(int u,int v) {
    if(!insert_query(u,v))return false;
    const auto pair=closest_pair(u,v);
    const int x=pair.x,y=pair.y;
    const int common=overlap(x,y);
    if(pair.path_edges.size()!=1) {
        int minimum=pair.path_edges.front();
        for(int id:pair.path_edges)
            if(edges_[id].weight<edges_[minimum].weight)minimum=id;
        cut(minimum);
        link(x,y,common);
    }
    int xy=-1;
    for(int id:nodes_[x].incident) {
        const auto& e=edges_[id];
        if((e.a==x&&e.b==y)||(e.b==x&&e.a==y)) {
            xy=id;
            break;
        }
    }
    if(xy<0)throw std::logic_error("A1 missing insertion tree edge");

    std::vector<unsigned char> marked(graph_.vertex_count(),0);
    for(int member:nodes_[x].members)marked[member]=1;
    std::vector<int> kz;
    kz.reserve(static_cast<std::size_t>(common+2));
    for(int member:nodes_[y].members)
        if(marked[member])kz.push_back(member);
    kz.push_back(u);
    kz.push_back(v);

    const bool drop_x=static_cast<int>(nodes_[x].members.size())==common+1;
    const bool drop_y=static_cast<int>(nodes_[y].members.size())==common+1;
    if(fast_) {
        if(drop_x)fast_->remove_clique(x);
        if(drop_y)fast_->remove_clique(y);
    }
    cut(xy);
    const int z=allocate_node(std::move(kz));
    link(x,z,common+1);
    link(z,y,common+1);
    if(drop_x)contract(z,x);
    if(drop_y)contract(z,y);
    graph_.add_edge(u,v);
    if(fast_) {
        add_to_fast_index(z);
        fast_->rebalance(static_cast<int>(graph_.edge_count()));
    }
    return true;
}

CliqueTree SparseIbarraChordalGraphA1::snapshot() const {
    CliqueTree tree;
    std::vector<int> index(nodes_.size(),-1);
    for(int id=0;id<static_cast<int>(nodes_.size());++id) {
        if(!nodes_[id].active)continue;
        index[id]=static_cast<int>(tree.cliques.size());
        tree.cliques.push_back(nodes_[id].members);
    }
    for(const auto& edge:edges_)
        if(edge.active)
            tree.edges.push_back({index[edge.a],index[edge.b],
                                  edge.weight,edge.weight==0});
    return tree;
}

} // namespace mtp
