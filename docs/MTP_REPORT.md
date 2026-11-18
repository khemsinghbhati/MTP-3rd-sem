# M.Tech Thesis Project — End-Semester Technical Report

**Project:** Chordal Graphs: Foundations, Maximal Cliques, Clique Trees, and Dynamic Algorithms  
**Student:** Khem Singh  
**Program:** M.Tech, Computer Science and Engineering, IIT Bhubaneswar  
**Report updated:** November 18, 2026  
**End-semester presentation:** November 30, 2026  
**Primary target literature:** Louis Ibarra (2008), *Fully Dynamic Algorithms for Chordal Graphs and Split Graphs*, ACM Transactions on Algorithms, 4(4), 40:1–40:20, DOI 10.1145/1383369.1383371.

> **Document status:** This is the single evolving technical report for the
> end-semester thesis presentation. It documents completed theory, implemented
> algorithms, formal arguments, testing, open proof obligations, and planned
> dynamic-graph modules. The project does **not** yet implement the complete
> dynamic algorithm. Complexity targets attributed to the research literature
> must be independently verified against the primary publications.

---

## Abstract

This thesis project investigates chordal graphs, perfect elimination orderings,
maximal clique structures, and their use in supporting edge updates while
preserving chordality. The target work of Ibarra (2008) studies maintenance of a clique-tree representation of a chordal
graph and queries for whether inserting or deleting an arbitrary edge keeps the
graph chordal. The **completed baseline** consists of: (i) a C++17 graph
representation, (ii) Maximum Cardinality Search (MCS) using weight-indexed
buckets for linear-time ordering, (iii) checking whether that ordering is a
perfect elimination ordering (PEO), and (iv) an algorithm that enumerates
maximal cliques of chordal graphs using later neighbors and a parent-based
subsumption rule. The refactored implementation has been checked against
independent brute-force oracles on all simple graphs with one through six
vertices and 600 additional random chordal instances with seven through twelve
vertices. Clique-graph construction, clique-tree construction, and dynamic edge
updates remain future implementation stages.

**Keywords:** chordal graph; perfect elimination ordering; maximum cardinality
search; maximal clique; clique tree; dynamic graph algorithm.

## 1. Problem motivation, scope and objectives

The subject of the thesis is algorithmic maintenance of structural properties
of undirected graphs under changes to their edge sets. A graph is *chordal* if
it has no induced cycle on four or more vertices. Chordal graphs admit strong
structural characterizations, including the existence of a perfect elimination
ordering, which makes maximal cliques amenable to efficient computation.

The research objective is a fully dynamic approach that maintains a
clique-tree representation while answering queries about whether individual
edge insertions or deletions preserve chordality. The following bounds are
provisional literature-level targets that require verification against the
original paper; they are **not** results achieved in this project:

| Literature-level operation / implementation | Provisional complexity target |
| --- | --- |
| First implementation: each operation | O(n) |
| Second: insertion query | O(log^2 n) |
| Second: insertion | O(n) |
| Second: deletion query | O(n) |
| Second: deletion | O(n log n) |
| Specialized deletion query | O(sqrt(m)) |
| Specialized data structure update | O(m) |

Here `n` is the number of vertices and `m` the number of edges. The above are
provisional until the exact preconditions and data structures have been
checked against the original paper before reproduction or comparison.

### 1.1 Current implementation baseline

The currently implemented foundation comprises:

1. Graph-theoretic preliminaries, including chordal graphs and PEOs.
2. Linear-time Maximum Cardinality Search (MCS) and its correctness argument.
3. Computing maximal cliques from a PEO and reasoning about subsumption.

Clique-graph construction, clique-tree construction, and fully dynamic edge
operations are tracked as remaining work, not completed results.

### 1.2 Project boundaries at this snapshot

- **Implemented:** simple undirected graph API; MCS; PEO verification;
  maximal-clique extraction; CLI; automated tests.
- **Not yet implemented:** clique graph `W_G`, clique tree `T`, insertion/deletion
  queries, structural updates, running-time experiments for dynamic operations.
- **No unsupported claim:** the code is a reorganized and validated baseline,
  not an original new dynamic graph algorithm.

## 2. Mathematical preliminaries

### 2.1 Graph and chordal graph

Let `G = (V, E)` be a finite undirected, simple graph, with `n = |V|` and
`m = |E|`. A **chord** of a cycle is an edge joining two vertices that are
nonconsecutive on the cycle. The graph is **chordal** if every cycle of length
at least four possesses a chord; equivalently it contains no chordless cycle
of length at least four.

Example: a four-cycle with no diagonal is non-chordal. Adding one diagonal
makes the graph chordal. This is why `data/nonchordal_cycle4.txt` is an
intentional negative test.

### 2.2 Clique and maximal clique

A **clique** is a vertex subset whose vertices are pairwise adjacent. It is
**maximal** if no strictly larger clique contains it. This is different from a
**maximum clique**, which has largest cardinality among all cliques.

In chordal graphs, a PEO makes it possible to construct a candidate clique
associated with each vertex and filter non-maximal candidates efficiently.

### 2.3 Perfect elimination ordering (PEO)

An ordering `alpha = (v_0, v_1, ..., v_{n-1})` is a **PEO** if for every vertex
`u`, the neighbors appearing *later* than `u` in `alpha` induce a clique. Write

```
N_plus(u) = { v in N(u) : position(v) > position(u) }.
```

Then the PEO condition is: `N_plus(u)` is a clique for all `u`.

**Foundational theorem:** An undirected graph is chordal
if and only if it has a perfect elimination ordering.

### 2.4 Why a PEO is useful

Because the later-neighbor set of any vertex forms a clique, the set

```
C_u = {u} union N_plus(u)
```

is also a clique. Therefore every vertex generates a candidate clique, from
which the maximal ones can be selected. This is the central bridge between
MCS/PEO and maximal-clique extraction.

## 3. Maximum Cardinality Search (MCS)

### 3.1 Algorithmic idea

MCS chooses an unnumbered
vertex with the greatest number of already-numbered neighbors. It writes
vertices into the PEO array **from right to left**.

Maintain `weight[v]`, the count of previously selected/numbered neighbors of
an unnumbered vertex `v`. Initially all weights are zero. At each stage:

1. Choose an unnumbered vertex with maximum weight.
2. Store it in the next free position working backward from `n-1` to `0`.
3. Mark it numbered.
4. Increment the weight of each still-unnumbered neighbor.

MCS is well defined for general undirected graphs, but the resulting ordering
is guaranteed to be a PEO only when the graph is chordal.

### 3.2 Bucket-based implementation

The implementation uses the following structures:

| Structure | Role |
| --- | --- |
| `weight[v]` | Count of already-numbered neighbors |
| `numbered[v]` | Whether vertex `v` is already selected |
| `buckets[w]` | Doubly linked list of unnumbered vertices of weight `w` |
| `locations[v]` | Iterator to the current bucket entry for vertex `v` |
| `max_weight` | Highest weight bucket currently considered |
| `ordering[i]` | Selected vertex placed from right to left |

The iterator array enables O(1) removal from a bucket; each weight increment
then becomes a constant-time removal/reinsertion. `std::list` is used because
it provides stable iterators for unaffected entries. The `push_front` behavior
in the baseline code has been preserved so the sample ordering is reproducible.

### 3.3 Pseudocode

```
MCS(G):
    for v in V:
        weight[v] = 0
        numbered[v] = false
        push v into buckets[0]
        remember iterator locations[v]
    max_weight = 0

    for position = n-1 down to 0:
        while buckets[max_weight] is empty:
            max_weight -= 1
        v = remove a vertex from buckets[max_weight]
        ordering[position] = v
        numbered[v] = true

        for each neighbor u of v:
            if not numbered[u]:
                erase u from buckets[weight[u]] using locations[u]
                weight[u] += 1
                insert u into buckets[weight[u]]
                update locations[u] and max_weight

    return ordering
```

### 3.4 Correctness statement and proof overview

**Theorem:** If `G` is chordal, the ordering produced by MCS is a PEO.

A proof overview, attributed to Vandenberghe and Andersen, has the following
structure:

1. Assume the ordering is not a PEO. One can then find a chordless path whose
   endpoints occur later in the ordering than its internal vertices.
2. Choose such a path maximizing the earlier endpoint position.
3. At the selection of the lower-positioned endpoint, compare its MCS weight
   with the weight of the first internal vertex. One endpoint is already
   numbered and contributes to the internal vertex's weight, forcing an
   additional already-numbered neighbor at the other endpoint.
4. This extra neighbor either closes a forbidden chordless cycle or produces
   a forbidden path with a strictly better endpoint-position value.
5. Both alternatives contradict the assumptions. Therefore the MCS ordering
   has the PEO property.

This is a **high-level proof sketch**, not an independently verified full
formal derivation. For the thesis submission, the exact quantifiers, ordering
conventions, and edge cases must be checked against the cited proof.

### 3.5 Complexity analysis

- Initialize weights, flags, and buckets: `O(n)`.
- Extract every vertex once: `O(n)`.
- Scan undirected adjacency entries over the whole execution: `2m` entries,
  hence `O(m)`.
- Each neighbor weight change costs O(1) with a saved list iterator.
- The `max_weight` variable may decrease across empty buckets, but every
  decrement is amortized against a preceding increase caused by an edge scan;
  total work for empty-bucket traversal is `O(m)`.

Thus the MCS ordering procedure is **O(n + m)**, using **O(n + m)** storage
including the graph. This claim refers to the ordering computation once the
graph has been constructed.

## 4. Verifying a perfect elimination ordering

The modular implementation includes an explicit PEO check before
maximal-clique extraction. This prevents non-chordal inputs from silently
producing invalid candidate cliques.

Given an ordering, compute each vertex's position. For each `u`, identify the
earliest later neighbor `p` (if any). Check that each other later neighbor of
`u` is adjacent to `p`. Performing this check for every vertex is sufficient
for the PEO property (using induction on the ordering's later vertices).

The graph implementation maintains `unordered_set` membership indices, so
checking adjacency is **O(1) expected time**. Consequently PEO verification
is **O(n + m) expected time** and **O(n)** additional position-array space,
with the hash adjacency index residing in `Graph`.

Incorrect-length or repeated-vertex orderings are rejected. On failure, the
CLI reports that the graph is not chordal and exits with status `2`.

**Engineering note:** As is standard for hash tables, the O(1) membership
cost is an average/expected guarantee, not a strict adversarial worst-case
bound. MCS itself retains the original deterministic O(n+m) bound.

## 5. Maximal clique extraction

### 5.1 Candidate cliques

For a PEO `alpha`, define the later-neighbor set

```
Madj(u) = N_plus(u)
C_u = {u} union Madj(u).
```

Since `Madj(u)` is a clique under a PEO, every candidate `C_u` is a clique.
Every **maximal** clique appears as one of these candidates:

Take any maximal clique `K`, and let `u` be its earliest vertex in the PEO.
All other vertices of `K` are later neighbors of `u`, so `K` is contained in
`C_u`. But `C_u` is a clique and `K` is maximal; therefore `K = C_u`. This establishes the maximal-clique candidate lemma.

### 5.2 Parent vertex

For `Madj(u) != empty`, let `p(u)` be the vertex of `Madj(u)` appearing
**earliest** in the PEO. Otherwise set `p(u) = -1`.

Because `Madj(u)` is a clique and `p(u)` is its earliest vertex,

```
Madj(u) \ {p(u)} subseteq Madj(p(u)).
```

### 5.3 The parent-subsumption condition

The implementation uses the following parent-subsumption rule:

```
if |Madj(u)| - 1 == |Madj(p(u))|:
    mark candidate C_(p(u)) as not maximal
```

For a particular `u` and its parent `p`, the relationship can be derived from
the inclusion in Section 5.2. If the sizes are equal, the sets are equal:

```
Madj(u) \ {p} = Madj(p)
C_p = Madj(u) proper_subset C_u.
```

Thus `C_p` is non-maximal. Conversely, if `C_p` is contained in `C_u` for
this parent-child relationship, the same set identity and size condition
follow. This establishes the **pairwise parent-subsumption equivalence**.

The report does not yet present a full proof that *every* non-maximal
candidate has such a witness among the tested parent-child relationships. That global-completeness argument should be included or
cited rigorously in the final thesis. The exhaustive oracle tests described
below are valuable evidence but do not substitute for the missing formal
argument.

### 5.4 Extraction algorithm

```
EXTRACT_MAXIMAL_CLIQUES(G, peo):
    verify peo is valid for G
    position = inverse permutation of peo

    for each vertex u:
        Madj(u) = neighbors appearing later than u
        parent[u] = earliest vertex in Madj(u), or -1
        later_degree[u] = |Madj(u)|

    maximal[u] = true for every u
    for each vertex u:
        p = parent[u]
        if p != -1 and later_degree[u] - 1 == later_degree[p]:
            maximal[p] = false

    result = []
    for u in peo order:
        if maximal[u]:
            result.append({u} union Madj(u))
    return result
```

### 5.5 Complexity

Computing `position`, `Madj`, `parent`, and sizes visits each vertex or edge
a constant number of times: `O(n+m)`. All parent-subsumption decisions take
`O(n)`. Materializing the output takes `O(S)`, where

```
S = sum over reported maximal cliques K of |K|.
```

Overall the implementation is `O(n + m + S)` **expected**, including its
hash-based PEO validation, and `O(n+m+S)` space including graph storage and
materialized output. For chordal graphs the total candidate later-neighbor
storage itself is bounded by `O(n+m)`.

The extraction algorithm is designed for linear work in the graph size;
writing an explicit `S` term in this report makes output construction cost clear.

## 6. Original seven-vertex example and dry run

The original graph (preserved in `data/example_7_vertices.txt`) has seven
vertices and ten undirected edges:

```
V = {0,1,2,3,4,5,6}
E = {(0,1),(1,3),(0,2),(2,3),(2,4),
     (2,5),(4,5),(3,6),(0,3),(1,2)}.
```

For the seven-vertex example, MCS follows this selection order (with the
specified bucket tie-breaking):

```
6 -> 3 -> 0 -> 2 -> 1 -> 5 -> 4
```

Because MCS numbers vertices backward, the resulting PEO is:

```
[4, 5, 1, 2, 0, 3, 6]
```

The refactored CLI prints the same ordering. Maximal cliques, expressed as
**sets** (vertex output order is not semantically important), are:

```
K1 = {0, 1, 2, 3}
K2 = {2, 4, 5}
K3 = {3, 6}
```

Interpretation: The four vertices of `K1` are pairwise connected; likewise
those of `K2` and `K3`. No listed clique is a strict subset of another; the
test oracle verifies that no other maximal clique exists in this instance.

## 7. Software implementation and code organization

### 7.1 Modular implementation baseline

The implementation separates graph representation, MCS/PEO, maximal-clique
extraction, and command-line input into dedicated components. It uses the
MCS bucket strategy, later-neighbor definition, and parent-subsumption
condition described earlier, with input validation and automated tests.

### 7.2 Module responsibilities

| Component | Header | Implementation | Responsibility |
| --- | --- | --- | --- |
| Graph | `include/mtp/graph.hpp` | `src/graph.cpp` | Undirected simple graph, validated edges and membership |
| MCS / PEO | `include/mtp/mcs.hpp` | `src/mcs.cpp` | Order vertices using MCS; verify PEO |
| Maximal cliques | `include/mtp/maximal_cliques.hpp` | `src/maximal_cliques.cpp` | Compute and filter candidates |
| CLI | - | `src/main.cpp` | Read graph text files and present results |
| Tests | - | `tests/test_algorithms.cpp` | Independent oracle checks |

The public algorithm functions accept a read-only `Graph` reference. This
keeps the algorithm stages separate from I/O. Future modules should follow
the same pattern instead of expanding a monolithic class.

### 7.3 Input contract and error handling

A graph file begins with `n m` followed by `m` undirected edges `u v`.
Vertices are numbered from zero. The loader rejects invalid headers, missing
edge rows, extra tokens, duplicate edges, out-of-range IDs, and self-loops.
A disconnected chordal graph is a valid input. A four-cycle without a chord
is rejected at the PEO-verification stage.

### 7.4 Compilation and execution

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/mtp_cli data/example_7_vertices.txt
ctest --test-dir build --output-on-failure
```

The project targets ISO C++17 and requires CMake 3.16 or later. Continuous
integration configuration is in `.github/workflows/ci.yml` and runs on push
and pull requests once published to GitHub. The workflow is not evidence of
a hosted run before the first GitHub push.

## 8. Verification and results

### 8.1 Test methodology

A test suite in `tests/test_algorithms.cpp` uses **independent small-graph
oracles** rather than only reusing the production algorithm:

- **Chordality oracle:** recursively removes a vertex whose remaining
  neighbors form a clique; dynamic programming over remaining-vertex subsets
  checks whether a full simplicial elimination sequence exists.
- **Maximal-clique oracle:** enumerates all nonempty vertex subsets and tests
  pairwise adjacency and whether an omitted vertex can extend each clique.
- **Production checks:** for every graph, compare MCS-produced PEO status to
  the chordality oracle; if chordal, compare normalized maximal-clique sets to
  the brute-force result; if non-chordal, require extraction to reject input.

The independent algorithmic oracles are intentionally exponential and are
used **only for small tests**. They do not replace the efficient production
implementation.

### 8.2 Test coverage and observed results

The test suite was compiled and executed in the development environment on
November 18, 2026. All checks below passed:

| Test | Scope | Observed result |
| --- | --- | --- |
| Example graph regression | 7 vertices, 10 edges | Expected PEO and three maximal cliques |
| Exhaustive graph coverage | Every simple graph with `n = 1..6` | PASS |
| Chordal instances within exhaustive coverage | 19,048 chordal graphs | PASS |
| Remaining non-chordal instances | Compared to chordality oracle | Correctly rejected |
| Random chordal coverage | 600 seeded instances, 7–12 vertices | PASS |
| Edge cases | Empty, isolated and disconnected graph; duplicate edge behavior; self-loop rejection; 4-cycle | PASS |

Total exhaustive graph inputs over `n=1..6`:

```
2^0 + 2^1 + 2^3 + 2^6 + 2^10 + 2^15 = 33,867.
```

Across these 33,867 inputs, 19,048 were chordal. The seeded random test
instances were generated by attaching each new vertex to a subset of an
existing clique, which preserves chordality. Seed: `20261118`.

**Limitations:** Exhaustive tests cover graphs with at most six vertices,
and seeded randomized tests cover selected cases at larger sizes. The
currently supplied suite does not establish memory use or runtime scaling for
very large graphs, and does not test any future dynamic update algorithm.

### 8.3 Reproducing the sample output

```text
MCS-derived ordering: 4 5 1 2 0 3 6
Maximal cliques (3):
{ 4 2 5 }
{ 1 0 3 2 }
{ 3 6 }
```

The CLI prints the PEO and the maximal-clique count. Vertex output order
within a clique does not change the mathematical result.

## 9. Thesis roadmap and future modules

The implementation roadmap is:

```text
[Chordal foundations]
        |
        v
[MCS + PEO] --------------------------- COMPLETE
        |
        v
[Maximal cliques] -------------------- COMPLETE
        |
        v
[Clique graph W_G] ------------------- NOT IMPLEMENTED
        |
        v
[Clique tree T] ---------------------- NOT IMPLEMENTED
        |
        v
[Dynamic edge queries and updates] -- NOT IMPLEMENTED
```

### 9.1 Next: clique graph `W_G`

The planned clique graph has vertices representing maximal cliques of
`G`. The **exact definition of its edges** and
any weights, labels, or supporting metadata for the intended algorithm must
be established from the target source before implementation. This report
does **not** invent that definition.

**Proposed new module:** `include/mtp/clique_graph.hpp`,
`src/clique_graph.cpp`, and corresponding tests.

Acceptance requirements: formal definition, construction algorithm,
correctness/complexity rationale, worked example, and test cases documented
here before marking complete.

### 9.2 Then: clique tree `T`

Investigate a clique-tree representation consistent with the target paper.
The standard clique-tree/running-intersection property is a useful
background criterion, but the specific representation, construction method,
and update rules to be implemented must be checked against the target
literature. Do not treat a generic clique-tree constructor as proof of
implementing Ibarra's dynamic data structure.

**Proposed new module:** `include/mtp/clique_tree.hpp`,
`src/clique_tree.cpp`, and corresponding tests.

### 9.3 Then: dynamic queries and updates

Study edge insertion and deletion separately. For each operation:

1. State its exact precondition and chordality-preservation criterion.
2. Implement a **query that makes no modification** to the graph.
3. Implement the permitted structural change and data structure update.
4. Validate graph and clique-tree invariants after the update.
5. Compare against a small-graph rebuild oracle and document complexity.

The target literature and any optimized data structures must be reviewed
before committing to specific data structures or runtime claims.

### 9.4 Suggested stage-by-stage development standard

A new stage is considered completed only when the following appear in this
single report: *definition → theorem/argument → algorithm → implementation →
small worked example → tests → complexity → limitations*. Keep tests and code
separate from the report.

## 10. Open questions and proof obligations

| Item | Why it matters | Status |
| --- | --- | --- |
| Confirm Ibarra's exact clique-graph edge definition | Necessary before implementing `W_G` | OPEN |
| Confirm required clique-tree representation | Determines data structure and invariants | OPEN |
| Supply/identify a full proof of global parent-rule filtering | Makes extraction's completeness argument rigorous | OPEN |
| Validate paper's stated complexity bounds from primary text | Avoids unverified runtime assumptions | OPEN |
| Implement dynamic insert/delete queries | Central thesis target | OPEN |
| Implement incremental updates and compare against rebuilding | Central thesis target | OPEN |
| Benchmark runtime/scaling once dynamics are implemented | Experimental evaluation | OPEN |

## 11. Development log

| Date | Milestone | Evidence |
| --- | --- | --- |
| 2026-11-18 | Refactored code into separate C++17 graph, MCS, and clique modules | `src/`, `include/` |
| 2026-11-18 | Added PEO verification, CLI input handling, tests, CMake, CI configuration | Source and test files |
| 2026-11-18 | Exhaustive verification for n ≤ 6 and 600 random chordal cases | `tests/test_algorithms.cpp` |
| Pending | Clique graph, clique tree, dynamic algorithm | Not claimed as completed |

**Reporting rule:** Keep milestone dates and achievements accurate. Clearly
separate implemented results, test evidence, open obligations, and proposed
algorithms for the end-semester evaluation.

## 12. Selected bibliography

These works inform the project's research direction; individual theoretical
or complexity claims must be checked against the relevant primary text before
they are presented as independently verified results:

1. Ibarra, L. (2008). *Fully Dynamic Algorithms for Chordal Graphs and Split
   Graphs*. ACM Transactions on Algorithms, 4(4), 40:1–40:20.
   DOI: `10.1145/1383369.1383371`.
2. Tarjan, R. E., and Yannakakis, M. (1984). *Simple Linear-Time Algorithms
   to Test Chordality of Graphs, Test Acyclicity of Hypergraphs, and
   Selectively Reduce Acyclic Hypergraphs*. SIAM Journal on Computing,
   13(3), 566–579.
3. Golumbic, M. C. (1980). *Algorithmic Graph Theory and Perfect Graphs*.
   Academic Press.
4. Vandenberghe, L., and Andersen, M. S. (2015). *Chordal Graphs and
   Semidefinite Optimization*. Foundations and Trends in Optimization,
   1(4), 241–433.
5. Gavril, F. (1972). *Algorithms for minimum coloring, maximum clique,
   minimum covering by cliques, and maximum independent set of a chordal
   graph*. SIAM Journal on Computing, 1(2), 180–187.
6. Blair, J. R. S., and Peyton, B. (1993). *An introduction to chordal
   graphs and clique trees*. In *Graph Theory and Sparse Matrix
   Computation*, pp. 1–29. Springer.

### Implementation evidence

- **Modular C++ implementation:** `include/mtp/` and `src/`.
- **Correctness and regression tests:** `tests/test_algorithms.cpp`.
- **Reproducible build and CI:** `CMakeLists.txt` and
  `.github/workflows/ci.yml`.

---

*End of current report snapshot. Future work should be appended to the
relevant sections of this same report, with explicit dates and evidence.*