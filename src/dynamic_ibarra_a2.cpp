#include "mtp/dynamic_ibarra_a2.hpp"
#include "mtp/mcs.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace mtp {

SleatorTarjanChordalGraphA2::SleatorTarjanChordalGraphA2(Graph graph)
    : data_(Graph(graph.vertex_count())), tree_(graph.vertex_count()+2) {
    // Ibarra §5.1: construct W using chordality-preserving edge insertions
    // beginning with n isolated vertices in reverse PEO order.
    const auto peo=maximum_cardinality_search(graph);
    if(!is_perfect_elimination_ordering(graph,peo))
        throw std::invalid_argument("A2 initial graph must be chordal");

    // Ibarra §2: the initial clique tree contains weight-zero dummy
    // edges joining disconnected singleton cliques.
    for(const auto& e:data_.edges_)
        if(e.active)tree_.link(e.a,e.b,e.weight);
    const int n=graph.vertex_count();
    vertex_witness_.assign(n,-1);
    for(int c=0;c<static_cast<int>(data_.nodes_.size());++c)
        if(data_.nodes_[c].active)
            for(int v=0;v<n;++v)
                if(data_.nodes_[c].contains[v])vertex_witness_[v]=c;
    for(int c:vertex_witness_)
        if(c<0)throw std::logic_error("missing singleton clique witness");
    initialize_weights();
    attach_tree_hooks();

    std::vector<int> position(n,-1);
    for(int i=0;i<n;++i)position[peo[i]]=i;
    for(int i=n-1;i>=0;--i) {
        const int u=peo[i];
        for(int v:graph.neighbors(u)) {
            if(position[v]<=i)continue;
            if(!insert_edge(u,v))
                throw std::logic_error("paper-specified PEO insertion rejected");
        }
    }
    link_count_last_=cut_count_last_=0;
}

void SleatorTarjanChordalGraphA2::attach_tree_hooks() {
    data_.tree_link_hook_=[this](int a,int b,int cost) {
        tree_.link(a,b,cost);
        ++link_count_last_;
    };
    data_.tree_cut_hook_=[this](int a,int b) {
        tree_.cut(a,b);
        ++cut_count_last_;
    };
}
void SleatorTarjanChordalGraphA2::initialize_weights() {
    // Ibarra §5.1: initial maximal cliques are singleton vertices.
    const int cap=static_cast<int>(data_.nodes_.size());
    W_.assign(cap,std::vector<int>(cap,0));
    for(int i=0;i<cap;++i)
        if(data_.nodes_[i].active)W_[i][i]=1;
}
void SleatorTarjanChordalGraphA2::check_dynamic_tree_invariants() {
    tree_.check_invariants(); // debug/test only, not part of operations
}
std::pair<int,int> SleatorTarjanChordalGraphA2::closest(int u,int v) {
    const int w=vertex_witness_.at(u),z=vertex_witness_.at(v);
    if(w<0||z<0||!data_.nodes_[w].active||!data_.nodes_[z].active||
       !data_.nodes_[w].contains[u]||!data_.nodes_[z].contains[v])
        throw std::logic_error("stale Ibarra A2 clique witnesses");

    // Ibarra (2008), Section 5, paragraph Insert-Query:
    // binary search the nearest u/v clique nodes on the w--z path.
    const int count=tree_.path_length(w,z);
    int left=0,right=count-1;
    while(left<right) {
        const int mid=(left+right+1)/2;
        const int candidate=tree_.kth_on_path(w,z,mid);
        if(data_.nodes_[candidate].contains[u])left=mid;
        else right=mid-1;
    }
    const int x=tree_.kth_on_path(w,z,left);
    left=0;right=count-1;
    while(left<right) {
        const int mid=(left+right)/2;
        const int candidate=tree_.kth_on_path(w,z,mid);
        if(data_.nodes_[candidate].contains[v])right=mid;
        else left=mid+1;
    }
    const int y=tree_.kth_on_path(w,z,left);
    return {x,y};
}
bool SleatorTarjanChordalGraphA2::insert_query(int u,int v) {
    if(u==v||data_.graph_.has_edge(u,v))return false;
    const auto [x,y]=closest(u,v);
    if(x==y)return false;
    // Ibarra, Theorem 6, constant-time stored clique intersection W.
    return tree_.min_on_path(x,y)==W_[x][y];
}
bool SleatorTarjanChordalGraphA2::delete_query(int u,int v) const {
    return data_.delete_query(u,v);
}
void SleatorTarjanChordalGraphA2::enable_fast_delete_queries() {
    data_.enable_fast_delete_queries();
}
const Graph& SleatorTarjanChordalGraphA2::graph() const noexcept {
    return data_.graph();
}
CliqueTree SleatorTarjanChordalGraphA2::snapshot() const {
    return data_.snapshot();
}

// Source-prescribed Ibarra §5.1 W matrix maintenance follows below.
// Keep the precise published split-and-join clique transformation logic.
bool SleatorTarjanChordalGraphA2::delete_edge(int u,int v) {
    if(!delete_query(u,v))return false;
    const int x=data_.clique_of_edge(u,v);
    const int k=data_.nodes_[x].size;
    const int cap=static_cast<int>(data_.nodes_.size());
    std::vector<bool> old_active(cap,false);
    for(int i=0;i<cap;++i)old_active[i]=data_.nodes_[i].active;
    const std::vector<int> old_row=W_[x];
    int absorb_u=-1,absorb_v=-1;
    for(int id:data_.nodes_[x].incident) {
        const auto& edge=data_.edges_[id];
        const int other=edge.a==x?edge.b:edge.a;
        const bool has_u=data_.nodes_[other].contains[u];
        const bool has_v=data_.nodes_[other].contains[v];
        if(edge.weight==k-1 && has_u)absorb_u=other;
        if(edge.weight==k-1 && has_v)absorb_v=other;
    }
    link_count_last_=cut_count_last_=0;
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
            W_[x][i]=W_[i][x]=value;
        }
        W_[x][x]=k-1;
    }
    if(x2>=0) {
        for(int i=0;i<cap;++i) {
            if(i==x||i==x2||!data_.nodes_[i].active)continue;
            const int value=old_row[i]-data_.nodes_[i].contains[u];
            W_[x2][i]=W_[i][x2]=value;
        }
        W_[x2][x2]=k-1;
        if(data_.nodes_[x].active)
            W_[x2][x]=W_[x][x2]=k-2;
    }
    return true;
}

// Ibarra (2008), Section 5.1: the new clique has a row of W
// derivable in O(1) per surviving clique, via the old x/y rows.
bool SleatorTarjanChordalGraphA2::insert_edge(int u,int v) {
    if(!insert_query(u,v))return false;
    // Section 3.4's actual insertion endpoints must also identify
    // the clique whose characteristic vector is reused by Section 4.
    // The query can use any nearest witness pair; insertion follows
    // the same endpoints as the maintained tree mutation.
    const auto pair=data_.closest_pair(u,v);
    const int x=pair.x,y=pair.y;
    const int cap=static_cast<int>(data_.nodes_.size());
    const int common=W_[x][y];
    const bool drop_x=data_.nodes_[x].size==common+1;
    const bool drop_y=data_.nodes_[y].size==common+1;
    // Ibarra Section 4: if a clique is subsumed, directly enlarge its
    // characteristic vector rather than introducing a temporary z node.
    // Keep the same choice as the linked clique-tree insertion routine.
    int reused=-1;
    if(drop_x&&!drop_y)reused=x;
    else if(drop_y&&!drop_x)reused=y;
    else if(drop_x&&drop_y)
        reused=data_.nodes_[x].incident.size()>=
               data_.nodes_[y].incident.size()?x:y;
    std::vector<bool> old_active(cap,false),on_x_side(cap,false);
    for(int i=0;i<cap;++i)old_active[i]=data_.nodes_[i].active;
    const auto old_x=W_[x];
    const auto old_y=W_[y];
    // An allowed insertion swaps the minimum-weight x--y path edge
    // for {x,y} (Ibarra, Theorem 6). The two resulting components
    // identify the side-specific Section 5.1 weight formula.
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

    link_count_last_=cut_count_last_=0;
    if(!data_.insert_edge(u,v))throw std::logic_error("insert precondition changed");
    int z=reused;
    if(z<0) {
        for(int i=0;i<cap;++i)
            if(data_.nodes_[i].active&&!old_active[i])z=i;
    }
    if(z<0||!data_.nodes_[z].active)
        throw std::logic_error("Ibarra new/reused maximal clique missing");
    for(int t=0;t<data_.graph_.vertex_count();++t) {
        const int old=vertex_witness_[t];
        if(data_.nodes_[old].active)continue;
        if(!data_.nodes_[z].contains[t])
            throw std::logic_error("invalid witness after contraction");
        vertex_witness_[t]=z;
    }
    for(int i=0;i<cap;++i) {
        if(i==z||!data_.nodes_[i].active)continue;
        W_[z][i]=W_[i][z]=new_row[i];
    }
    W_[z][z]=data_.nodes_[z].size;
    return true;
}
} // namespace mtp
