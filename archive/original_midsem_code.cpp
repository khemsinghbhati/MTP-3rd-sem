#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
using namespace std;
class ChordalGraph {
int V;
vector<vector<int>> adj;
public:
ChordalGraph(int vertices) : V(vertices) {
adj.resize(V);
}
void addEdge(int u, int v) {
adj[u].push_back(v);
adj[v].push_back(u);
}
// Step 1: Optimized O(V + E) Maximum Cardinality Search
vector<int> maximumCardinalitySearch() {
vector<int> peo(V);
vector<int> weight(V, 0);
vector<bool> numbered(V, false);
// buckets[w] stores all unnumbered nodes with weight 'w'
vector<list<int>> buckets(V);
// node_it[v] stores the iterator to node 'v' in its current bucket
// This allows O(1) removal from the middle of a list
vector<list<int>::iterator> node_it(V);

// Initially, all nodes have weight 0, so put them in buckets[0]
for (int i = 0; i < V; ++i) {
buckets[0].push_front(i);
node_it[i] = buckets[0].begin();
}
int max_weight = 0;
// Number vertices from V-1 down to 0
for (int i = V - 1; i >= 0; --i) {
// Find the highest non-empty bucket
while (buckets[max_weight].empty()) {
max_weight--;
}
// Extract the node from the max_weight bucket
int next_node = buckets[max_weight].front();
buckets[max_weight].pop_front();
peo[i] = next_node;
numbered[next_node] = true;
// Update the weights of unnumbered neighbors
for (int neighbor : adj[next_node]) {
if (!numbered[neighbor]) {
// Remove from current weight bucket in O(1)
buckets[weight[neighbor]].erase(node_it[neighbor]);
// Increment weight
weight[neighbor]++;
// Insert into new weight bucket in O(1)
buckets[weight[neighbor]].push_front(neighbor);
node_it[neighbor] = buckets[weight[neighbor]].begin();
// Update max_weight if necessary
if (weight[neighbor] > max_weight) {
max_weight = weight[neighbor];
}
}
}
}
return peo;
}

// Step 2 & 3: Fully Optimized O(V + E) Maximal Clique Extraction
vector<vector<int>> getMaximalCliques() {
// 1. Get the PEO using the optimized Maximum Cardinality Search
vector<int> peo = maximumCardinalitySearch();
vector<int> position(V);
for (int i = 0; i < V; ++i) {
position[peo[i]] = i;
}
vector<vector<int>> madj(V);
vector<int> parent(V, -1);
vector<int> madj_size(V, 0);
// 2. Find later neighbors (Madj) and the "parent" for each node
for (int u = 0; u < V; ++u) {
int earliest_later_neighbor = -1;
int min_pos = V; // A value larger than any possible position
for (int neighbor : adj[u]) {
// If the neighbor appears LATER in the PEO
if (position[neighbor] > position[u]) {
madj[u].push_back(neighbor);
// Track the earliest appearing later neighbor
if (position[neighbor] < min_pos) {
min_pos = position[neighbor];
earliest_later_neighbor = neighbor;
}
}
}
parent[u] = earliest_later_neighbor;
madj_size[u] = madj[u].size();
}
// 3. The O(V) Maximality Check
// Assume all candidate cliques are maximal until proven otherwise
vector<bool> is_maximal(V, true);
for (int u = 0; u < V; ++u) {
int p = parent[u];
if (p != -1) {
// If u's clique perfectly subsumes p's clique, p is not maximal

if (madj_size[u] - 1 == madj_size[p]) {
is_maximal[p] = false;
}
}
}
// 4. Collect the surviving cliques
vector<vector<int>> maximal_cliques;
for (int i = 0; i < V; ++i) {
int u = peo[i];
// Only build the clique if it survived the maximality check
if (is_maximal[u]) {
vector<int> clique = {u};
clique.insert(clique.end(), madj[u].begin(), madj[u].end());
maximal_cliques.push_back(clique);
}
}
return maximal_cliques;
}
};
int main() {
// Recreating a simple chordal graph
// Example: A square with a diagonal (Vertices: 0, 1, 2, 3)
ChordalGraph g(7);
g.addEdge(0, 1);
g.addEdge(1, 3);
g.addEdge(0, 2);
g.addEdge(2, 3);
g.addEdge(2,4);
g.addEdge(2,5);
g.addEdge(4,5);
g.addEdge(3,6);
g.addEdge(0, 3);
g.addEdge(1,2);
vector<vector<int>> max_cliques = g.getMaximalCliques();
cout << "Maximal Cliques:\n";
for (const auto& clique : max_cliques) {
cout << "{ ";
for (int node : clique) {

cout << node << " ";
}
cout << "}\n";
}
return 0;
}