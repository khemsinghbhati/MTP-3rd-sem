# Ibarra A1 — sparse exact implementation and complexity certificate

## Scope and method provenance

This branch implements the **first** fully dynamic chordal graph algorithm
from Ibarra (2008), Sections 3–4, through
`mtp::SparseIbarraChordalGraphA1`. The algorithm's insertion and deletion
criteria, clique-tree edge exchange, clique split, maximal-clique absorption,
and separator weights are unchanged. Its *representation* is changed from
n-bit characteristic vectors per clique to incidence lists.

References:

- Louis Ibarra, *Fully Dynamic Algorithms for Chordal Graphs and Split
  Graphs*, ACM Transactions on Algorithms 4(4), 2008,
  https://doi.org/10.1145/1383369.1383371 .
- Blair and Peyton, *An introduction to chordal graphs and clique trees*,
  1993, algorithm of maximum cardinality search and clique-tree
  construction.
- Tarjan and Yannakakis, *Simple linear-time algorithms to test chordality
  of graphs, test acyclicity of hypergraphs, and selectively reduce acyclic
  hypergraphs*, SIAM J. Comput. 13(3), 1984.
- Briggs and Torczon, *An efficient representation for sparse sets*, ACM
  LOPLAS 2(1–4), 1993, supporting precedent for using
  inverse-incidence and scratch markings rather than all-pairs tables.

This integration is an implementation-level composition of published
algorithms and standard linked-list operations, **not a claim that
Ibarra explicitly described this sparse data layout**.

## Data representation

- Each active clique-node `x` stores its actual members `K_x` as an
  unsorted array of vertex IDs (no `n`-length characteristic vector).
- `nodes_of[v]` is a doubly linked list of all clique nodes containing `v`.
  Each member-array entry stores its iterator into the inverse list,
  supporting constant-time removal.
- The clique tree uses adjacency lists with iterator-backed constant-time
  link/cut.
- Only O(n)-sized temporary mark arrays are initialized during operations.
- Node IDs are drawn from a fixed n+2-capacity arena and a free list.
- The graph is represented by adjacency vectors plus deterministic balanced
  adjacency sets. The `Graph::from_edges` input path bucket-orders adjacency
  lists by neighbor ID and uses the sorted-range C++17 `std::set`
  constructor, avoiding O(m log n) incremental input insertion.

A2 and its research indexed baseline continue using the existing dense
`DynamicChordalGraph` class and are **not** silently modified.

## Preprocessing complexity: O(n + m) worst case

Let `n=|V|`, `m=|E|`, and let the maximal cliques be `K_1,...,K_c`.
The chordal graph has `c<=n`. In a PEO, every maximal clique
has a distinct earliest vertex `v_i`; it is contained in
`{v_i} union N^+(v_i)`, where `N^+` is the set of later neighbors.
Therefore:

```math
\sum_{i=1}^c |K_i|\le\sum_{v\in V}(1+|N^+(v)|)=n+m.
```

MCS, PEO validation, Blair–Peyton clique-tree construction, and sparse
incidence initialization therefore take O(n+m) total worst-case time
and O(n+m) space for the *initial* chordal graph. `Graph::from_edges`
also costs O(n+m), under the standard C++17 sorted-set-construction
complexity guarantee. Loading the same input through m separate calls
to `Graph::add_edge` costs O(m log n) and is not the certified bulk path.

## Worst-case O(n) dynamic operations (not amortized)

- **Delete-Query:** test graph adjacency in O(log n), mark `T_u` and
  scan `T_v` to count common clique nodes in O(n).
- **Insert-Query:** multi-source BFS from all clique nodes containing
  `u` to the closest clique node containing `v`, in O(c)=O(n);
  collect the unique path's minimum separator weight in O(n);
  compute `|K_x intersect K_y|` by marking members of `K_x`
  and scanning members of `K_y`, in O(n); apply Ibarra's
  Theorem 6 criterion, unchanged.
- **Delete:** one Delete-Query plus O(n) clique-tree neighbor
  reclassification using batch membership marks; split `K_x`
  into `K_x-{v}` and `K_x-{u}`, update inverse list memberships
  in O(n), and perform O(n) constant-time link/cut/contraction steps.
  Graph edge removal scans two adjacency arrays of at most n elements.
- **Insert:** one Insert-Query, repeat nearest-pair lookup in O(n),
  execute Ibarra's path-minimum edge exchange, build
  `(K_x intersect K_y) union {u,v}` in O(n) and merge
  subsumed cliques with at most O(n) constant-time link/cuts.
  Graph edge addition costs O(log n), with worst-case
  O(n) vector reallocation.

All table and membership operations in the four core methods are
deterministic, and every loop iterates O(n) entries at most.
The program does **not** rely on expected-time hash lookups,
splay-tree amortization, or prezeroed n-by-n memory.

The optional `enable_fast_delete_queries` keeps Ibarra's **A1+**
index with the paper's *separate* quadratic / O(mn+n^2) preprocessing
accounting; enabling it is not part of A1's O(n+m) constructor.

## Audit and regression evidence

- Independent test against the original dense A1 oracle
  (`tests/test_sparse_a1_stress.cpp`).
- Exhaustive chordal graphs through five vertices and long randomized
  insertion/deletion sequences with independent PEO and
  maximal-clique/click-tree invariant checks
  (`tests/test_sparse_ibarra_a1.cpp`).
- Same exhaustive and random sequences with optional fast
  deletion-query index (`tests/test_sparse_ibarra_a1_fast.cpp`).
- Empty graph `n=3000`: exactly n clique-incidence entries, not n².
- Stars and large complete graphs under repeated updates.
- Bulk input loader tested against incremental adjacency,
  including duplicate rejection.

Passing CI establishes tested functional behavior; the complexity
arguments above establish worst-case running-time bounds only
under standard RAM/cost assumptions. `snapshot()` is an
untimed debug/export function and intentionally may enumerate O(n+m)
memberships.

## A2 policy

The original Ibarra A2 Section 7 claimed
`O(m+n log n)` preprocessing, but the complete
implementation (including W) does **not** achieve that.
Leave A2 labeled as **not achieved as published** for
preprocessing. Preserve its best verified implementation
and the separate R1 optimization work without altering
the published A2 operation targets.
