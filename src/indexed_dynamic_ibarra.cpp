#include "mtp/indexed_dynamic_ibarra.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace mtp {

void IndexedDynamicChordalGraph::HeavyPathIndex::build(
    const DynamicChordalGraph& data) {
    const int capacity = static_cast<int>(data.nodes_.size());
    parent.assign(capacity,-1);
    depth.assign(capacity,0);
    heavy.assign(capacity,-1);
    head.assign(capacity,-1);
    position.assign(capacity,-1);
    subtree_size.assign(capacity,0);
    parent_weight.assign(capacity,infinity);
    by_position.clear();

    std::vector<std::vector<std::pair<int,int>>> adjacency(capacity);
    int root = -1;
    int active = 0;
    for (int i=0;i<capacity;++i) {
        if (!data.nodes_[i].active) continue;
        ++active;
        if (root==-1) root=i;
    }
    for (const auto& e : data.edges_) {
        if (!e.active) continue;
        adjacency[e.a].push_back({e.b,e.weight});
        adjacency[e.b].push_back({e.a,e.weight});
    }
    if (root==-1) {
        segment.assign(2,infinity);
        base=1;
        return;
    }

    // An iterative DFS gives each parent exactly once.
    std::vector<int> order, stack{root};
    parent[root]=root;
    order.reserve(active);
    while(!stack.empty()) {
        const int u=stack.back();
        stack.pop_back();
        order.push_back(u);
        for (const auto [v,w] : adjacency[u]) {
            if (parent[v]!=-1) continue;
            parent[v]=u;
            depth[v]=depth[u]+1;
            parent_weight[v]=w;
            stack.push_back(v);
        }
    }
    if (static_cast<int>(order.size())!=active)
        throw std::logic_error("clique tree is not connected");

    // Size partition (Sleator-Tarjan 1983, Section 5).
    for(auto it=order.rbegin();it!=order.rend();++it) {
        const int u=*it;
        subtree_size[u]=1;
        int largest=0;
        for (const auto [v,w] : adjacency[u]) {
            (void)w;
            if(parent[v]!=u || v==u) continue;
            subtree_size[u]+=subtree_size[v];
            if(subtree_size[v]>largest) {
                largest=subtree_size[v];heavy[u]=v;
            }
        }
    }

    // Flatten heavy paths. Every light-edge step halves subtree size.
    std::vector<std::pair<int,int>> work{{root,root}};
    while(!work.empty()) {
        const auto [first,chain_head]=work.back();
        work.pop_back();
        for(int u=first;u!=-1;u=heavy[u]) {
            head[u]=chain_head;
            position[u]=static_cast<int>(by_position.size());
            by_position.push_back(u);
            for(const auto [v,w]:adjacency[u]) {
                (void)w;
                if(parent[v]==u && v!=u && v!=heavy[u])
                    work.push_back({v,v});
            }
        }
    }

    base=1;
    while(base<active) base*=2;
    segment.assign(base*2,infinity);
    for(int u:order) segment[base+position[u]]=parent_weight[u];
    for(int p=base-1;p>=1;--p)
        segment[p]=std::min(segment[2*p],segment[2*p+1]);
}

int IndexedDynamicChordalGraph::HeavyPathIndex::lca(int a,int b) const {
    while(head[a]!=head[b]) {
        if(depth[head[a]]>depth[head[b]]) a=parent[head[a]];
        else b=parent[head[b]];
    }
    return depth[a]<depth[b]?a:b;
}

int IndexedDynamicChordalGraph::HeavyPathIndex::ancestor(
    int v,int distance) const {
    if(distance<0 || distance>depth[v])
        throw std::out_of_range("ancestor distance outside tree");
    while(distance>0) {
        const int available=depth[v]-depth[head[v]];
        if(distance<=available) return by_position[position[v]-distance];
        distance-=available+1;
        v=parent[head[v]];
    }
    return v;
}

int IndexedDynamicChordalGraph::HeavyPathIndex::path_length(
    int a,int b) const {
    const int common=lca(a,b);
    return depth[a]+depth[b]-2*depth[common]+1;
}

int IndexedDynamicChordalGraph::HeavyPathIndex::kth(
    int a,int b,int k) const {
    const int ancestor_node=lca(a,b);
    const int up=depth[a]-depth[ancestor_node];
    const int down=depth[b]-depth[ancestor_node];
    if(k<0 || k>up+down) throw std::out_of_range("path index");
    if(k<=up) return ancestor(a,k);
    return ancestor(b,up+down-k);
}

int IndexedDynamicChordalGraph::HeavyPathIndex::segment_min(
    int first,int last) const {
    int answer=infinity;
    if(first>last) return answer;
    int lo=base+first,hi=base+last;
    while(lo<=hi) {
        if(lo&1)answer=std::min(answer,segment[lo++]);
        if(!(hi&1))answer=std::min(answer,segment[hi--]);
        lo/=2;hi/=2;
    }
    return answer;
}

int IndexedDynamicChordalGraph::HeavyPathIndex::min_on_path(
    int a,int b) const {
    int answer=infinity;
    while(head[a]!=head[b]) {
        if(depth[head[a]]<depth[head[b]]) std::swap(a,b);
        answer=std::min(answer,
            segment_min(position[head[a]],position[a]));
        a=parent[head[a]];
    }
    if(depth[a]<depth[b]) std::swap(a,b);
    answer=std::min(answer,segment_min(position[b]+1,position[a]));
    return answer;
}

IndexedDynamicChordalGraph::IndexedDynamicChordalGraph(Graph graph)
    : data_(Graph(graph.vertex_count())) {
    // Ibarra (2008), Section 5.1, explicitly describes an O(mn)
    // preprocessing alternative for W: start with an empty chordal graph
    // and insert edges incident to each vertex in reverse PEO order.
    // Every intermediate subgraph retains the same PEO.
    const auto peo=maximum_cardinality_search(graph);
    if(!is_perfect_elimination_ordering(graph,peo))
        throw std::invalid_argument("initial graph must be chordal");
    const int n=graph.vertex_count();
    vertex_witness_.assign(n,-1);
    const int capacity=static_cast<int>(data_.nodes_.size());
    for(int c=0;c<capacity;++c)
        if(data_.nodes_[c].active)
            for(int v=0;v<n;++v)
                if(data_.nodes_[c].contains[v])vertex_witness_[v]=c;
    for(int c:vertex_witness_)
        if(c<0)throw std::logic_error("missing singleton clique witness");
    initialize_weights();
    refresh_paths();

    std::vector<int> position(n,-1);
    for(int i=0;i<n;++i)position[peo[i]]=i;
    for(int i=n-1;i>=0;--i) {
        const int u=peo[i];
        for(int v:graph.neighbors(u)) {
            if(position[v]<=i)continue;
            if(!insert_edge(u,v))
                throw std::logic_error("PEO-driven insertion rejected");
        }
    }
}

void IndexedDynamicChordalGraph::initialize_weights() {
    // The initial graph is empty: its n maximal cliques are singletons,
    // so W is the identity matrix (Ibarra Section 5.1 initialization).
    // Allocate its characteristic n-by-n matrix in O(n^2).
    const int cap=static_cast<int>(data_.nodes_.size());
    intersection_weight_.assign(cap,std::vector<int>(cap,0));
    for(int i=0;i<cap;++i)
        if(data_.nodes_[i].active)
            intersection_weight_[i][i]=1;
}
void IndexedDynamicChordalGraph::refresh_paths() { paths_.build(data_); }

std::pair<int,int> IndexedDynamicChordalGraph::closest(
    int u,int v) const {
    const int w=vertex_witness_[u],z=vertex_witness_[v];
    if(w<0||z<0||!data_.nodes_[w].contains[u]||
       !data_.nodes_[z].contains[v])
        throw std::logic_error("stale vertex-clique witness");
    const int count=paths_.path_length(w,z);
    // By the induced-subtree property, u-membership is a prefix of
    // the w--z path and v-membership is a suffix of the same path.
    int left=0,right=count-1;
    while(left<right) {
        const int mid=(left+right+1)/2;
        if(data_.nodes_[paths_.kth(w,z,mid)].contains[u])left=mid;
        else right=mid-1;
    }
    const int x=paths_.kth(w,z,left);
    left=0;right=count-1;
    while(left<right) {
        const int mid=(left+right)/2;
        if(data_.nodes_[paths_.kth(w,z,mid)].contains[v])right=mid;
        else left=mid+1;
    }
    const int y=paths_.kth(w,z,left);
    return {x,y};
}

bool IndexedDynamicChordalGraph::insert_query(int u,int v) const {
    if(u==v || data_.graph_.has_edge(u,v)) return false;
    const auto [x,y]=closest(u,v);
    if(x==y)return false;
    return paths_.min_on_path(x,y)==intersection_weight_[x][y];
}
bool IndexedDynamicChordalGraph::delete_query(int u,int v) const {
    return data_.delete_query(u,v);
}
void IndexedDynamicChordalGraph::enable_fast_delete_queries() {
    data_.enable_fast_delete_queries();
}
const Graph& IndexedDynamicChordalGraph::graph() const noexcept {
    return data_.graph();
}
CliqueTree IndexedDynamicChordalGraph::snapshot() const {
    return data_.snapshot();
}

// Ibarra Section 5.1: after deleting {u,v}, split K_x into
// K_x-{v} and K_x-{u}, each intersection with an existing K_w
// changes by subtracting membership of v or u respectively.
bool IndexedDynamicChordalGraph::delete_edge(int u,int v) {
    if(!delete_query(u,v))return false;
    const int x=data_.clique_of_edge(u,v);
    const int k=data_.nodes_[x].size;
    const int cap=static_cast<int>(data_.nodes_.size());
    std::vector<bool> old_active(cap,false);
    for(int i=0;i<cap;++i)old_active[i]=data_.nodes_[i].active;
    const std::vector<int> old_row=intersection_weight_[x];
    int absorb_u=-1,absorb_v=-1;
    for(int id:data_.nodes_[x].incident) {
        const auto& edge=data_.edges_[id];
        const int other=edge.a==x?edge.b:edge.a;
        const bool has_u=data_.nodes_[other].contains[u];
        const bool has_v=data_.nodes_[other].contains[v];
        if(edge.weight==k-1 && has_u)absorb_u=other;
        if(edge.weight==k-1 && has_v)absorb_v=other;
    }
    if(!data_.delete_edge(u,v))throw std::logic_error("delete precondition changed");

    int x2=-1;
    for(int i=0;i<cap;++i)
        if(data_.nodes_[i].active&&!old_active[i])x2=i;
    // The old clique can survive as xu; xv may be new or absorbed.
    for(int t=0;t<data_.graph_.vertex_count();++t) {
        if(vertex_witness_[t]!=x)continue;
        const int replacement=(t==v)
            ? (x2>=0?x2:absorb_v)
            : (data_.nodes_[x].active?x:absorb_u);
        if(replacement<0 || !data_.nodes_[replacement].active ||
           !data_.nodes_[replacement].contains[t])
            throw std::logic_error("delete witness update failed");
        vertex_witness_[t]=replacement;
    }
    if(data_.nodes_[x].active) {
        for(int i=0;i<cap;++i) {
            if(i==x||i==x2||!data_.nodes_[i].active)continue;
            const int value=old_row[i]-data_.nodes_[i].contains[v];
            intersection_weight_[x][i]=intersection_weight_[i][x]=value;
        }
        intersection_weight_[x][x]=k-1;
    }
    if(x2>=0) {
        for(int i=0;i<cap;++i) {
            if(i==x||i==x2||!data_.nodes_[i].active)continue;
            const int value=old_row[i]-data_.nodes_[i].contains[u];
            intersection_weight_[x2][i]=intersection_weight_[i][x2]=value;
        }
        intersection_weight_[x2][x2]=k-1;
        if(data_.nodes_[x].active)
            intersection_weight_[x2][x]=intersection_weight_[x][x2]=k-2;
    }
    refresh_paths();
    return true;
}

// Ibarra (2008), Section 5.1: the new clique has a row of W
// derivable in O(1) per surviving clique, via the old x/y rows.
bool IndexedDynamicChordalGraph::insert_edge(int u,int v) {
    if(!insert_query(u,v))return false;
    const auto [x,y]=closest(u,v);
    const int cap=static_cast<int>(data_.nodes_.size());
    const int common=intersection_weight_[x][y];
    const bool drop_x=data_.nodes_[x].size==common+1;
    const bool drop_y=data_.nodes_[y].size==common+1;
    std::vector<bool> old_active(cap,false),on_x_side(cap,false);
    for(int i=0;i<cap;++i)old_active[i]=data_.nodes_[i].active;
    const auto old_x=intersection_weight_[x];
    const auto old_y=intersection_weight_[y];
    // An allowed insertion swaps the minimum-weight x--y path edge
    // for {x,y} (Ibarra, Theorem 6). The two resulting components
    // identify the side-specific Section 5.1 weight formula.
    const auto pair=data_.closest_pair(u,v);
    int cut_id=-1;
    if(pair.path_edges.size()==1)cut_id=pair.path_edges[0];
    else {
        for(int id:pair.path_edges)
            if(cut_id<0||data_.edges_[id].weight<data_.edges_[cut_id].weight)
                cut_id=id;
    }
    if(cut_id<0)throw std::logic_error("missing path minimum");
    std::vector<int> stack{x};
    on_x_side[x]=true;
    while(!stack.empty()) {
        const int node=stack.back();stack.pop_back();
        for(int id:data_.nodes_[node].incident) {
            if(id==cut_id)continue;
            const auto& edge=data_.edges_[id];
            const int next=edge.a==node?edge.b:edge.a;
            if(on_x_side[next])continue;
            on_x_side[next]=true;
            stack.push_back(next);
        }
    }
    if(on_x_side[y])throw std::logic_error("tree cut did not separate endpoints");

    // Precompute all new weights while the original row values and
    // characteristic vectors are still available.
    std::vector<int> new_row(cap,0);
    for(int i=0;i<cap;++i) {
        if(!old_active[i])continue;
        if(drop_x) new_row[i]=old_x[i]+data_.nodes_[i].contains[v];
        else if(drop_y) new_row[i]=old_y[i]+data_.nodes_[i].contains[u];
        else if(on_x_side[i])
            new_row[i]=old_y[i]+data_.nodes_[i].contains[u];
        else new_row[i]=old_x[i]+data_.nodes_[i].contains[v];
    }

    if(!data_.insert_edge(u,v))throw std::logic_error("insert precondition changed");
    int z=-1;
    for(int i=0;i<cap;++i)
        if(data_.nodes_[i].active&&!old_active[i])z=i;
    if(z<0)throw std::logic_error("new maximal clique not created");
    for(int t=0;t<data_.graph_.vertex_count();++t) {
        const int old=vertex_witness_[t];
        if(data_.nodes_[old].active)continue;
        if(!data_.nodes_[z].contains[t])
            throw std::logic_error("invalid witness after contraction");
        vertex_witness_[t]=z;
    }
    for(int i=0;i<cap;++i) {
        if(i==z||!data_.nodes_[i].active)continue;
        intersection_weight_[z][i]=intersection_weight_[i][z]=new_row[i];
    }
    intersection_weight_[z][z]=data_.nodes_[z].size;
    refresh_paths();
    return true;
}
} // namespace mtp
