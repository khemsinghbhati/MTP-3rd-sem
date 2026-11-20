# M.Tech Thesis Project — End-Semester Technical Report

> **Main-branch status (November 20, 2026):** This report covers A1,
> A1+, the split-graph algorithm, and the source-backed A2/A2+
> globally biased Sleator–Tarjan implementation. Section 14 preserves
> the historical O(n) insertion audit failure; **Section 15 documents
> the corrected O(n) worst-case insertion via BST85 linear rebuilding
> of the SAME ST83 forest**. Ibarra did not explicitly prescribe
> the full-forest rebuild schedule, so that source-supported refinement
> is disclosed rather than presented as literal pseudocode.
> **Important:** Sections 1–14 contain dated historical audit snapshots and
> must not be interpreted as overriding Section 15. Section 16 (the newest
> source audit) proves that the literal dense representation cannot meet the
> Section 7 preprocessing-table bounds and documents the published methods
> checked. No source-faithful faster preprocessing implementation is certified.

**Project:** Chordal Graphs: Foundations, Maximal Cliques, Clique Trees, and Dynamic Algorithms  
**Student:** Khem Singh  
**Program:** M.Tech, Computer Science and Engineering, IIT Bhubaneswar  
**Report updated:** November 20, 2026  
**End-semester presentation:** November 30, 2026  
**Primary target literature:** Louis Ibarra (2008), *Fully Dynamic Algorithms for Chordal Graphs and Split Graphs*, ACM Transactions on Algorithms, 4(4), 40:1–40:20, DOI 10.1145/1383369.1383371.

> **Document status:** This is the single evolving technical report for the
> end-semester thesis presentation. It documents completed theory, implemented
> algorithms, formal arguments, testing, open proof obligations, and planned
> dynamic-graph modules. The project now implements Ibarra's **first** dynamic chordal-graph
> algorithm and his constant-time split-graph degree-sequence algorithm.
> His optimized chordal-graph variants (Sections 5–6) remain outstanding.

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
vertices. A **direct, deterministic linear-time clique-tree constructor**
is now used by the production CLI. The explicit weighted clique intersection
graph and maximum-spanning-tree construction remain as a separate correctness
baseline, **not** as production preprocessing. Fully dynamic edge updates
remain unfinished.

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
literature-level bounds stated in Ibarra (2008), Section 1.3 (paper page
40:3). They are **claims of the paper**, not performance results achieved
by our implementation:

| Literature-level operation / implementation | Complexity in paper |
| --- | --- |
| First implementation: each operation | $O(n)$ |
| Second: insertion query | $O(\log^2 n)$ |
| Second: insertion | $O(n)$ |
| Second: deletion query | $O(n)$ |
| Second: deletion | $O(n\log n)$ |
| Specialized deletion query | $O(\sqrt m)$ |
| Specialized data structure update | $O(m)$ |

Here $n=|V|$ is the number of vertices and $m=|E|$ the number of edges.
The article describes these bounds as worst-case unless specified otherwise.
The code now implements the first four O(n) dynamic chordal operations,
with clique characteristic vectors and incremental tree updates. The
Sections 5–6 optimized chordal variants are not yet implemented.

### 1.1 Current implementation baseline

The currently implemented foundation comprises:

1. Graph-theoretic preliminaries, including chordal graphs and PEOs.
2. Linear-time Maximum Cardinality Search (MCS) and its correctness argument.
3. Computing maximal cliques from a PEO and reasoning about subsumption.
4. Constructing a clique tree directly from the PEO in worst-case
   $O(n+m)$ time, without enumerating the weighted clique graph.
5. Keeping an explicit clique graph and maximum-spanning-tree method
   **only as a test baseline**, not as the production algorithm.

**Remaining work:** published worst-case dynamic-tree implementation from
Section 5, combination into A2+, and full performance evaluation.

### 1.2 Project boundaries at this snapshot

- **Implemented:** simple undirected graph API; MCS; deterministic
  $O(n+m)$ PEO verification; maximal-clique extraction; direct
  $O(n+m)$ static clique-tree construction; CLI; automated tests.
  The explicit clique graph remains as a reference test implementation.
- **Not yet implemented:** Section 5 dynamic-tree / O(log² n) insertion
  query and full scaling benchmarks. Section 6 fast deletion is implemented.
- **No unsupported claim:** the code is a reorganized and validated baseline,
  not an original new dynamic graph algorithm.

## 2. Mathematical preliminaries

### 2.1 Graph and chordal graph

Let the input be a simple, undirected graph:

```math
G=(V,E), \qquad n=|V|, \qquad m=|E|.
```

Here, $V$ is the set of vertices, $E$ is the edge set, $n$ is the number
of vertices, and $m$ is the number of edges.

A **chord** is an edge joining two nonconsecutive vertices of a cycle.
The graph is **chordal** if every cycle of length at least $4$ has a chord.
Equivalently, there is **no induced chordless cycle of length $4$ or more**.

**Example:** The four-cycle $0\!-\!1\!-\!2\!-\!3\!-\!0$ has no chord and is
non-chordal. Adding $\{0,2\}$ makes that graph chordal. The negative input
file in the data folder exercises this distinction.

### 2.2 Clique, maximal clique, and maximum clique

A subset $K\subseteq V$ is a **clique** when its distinct vertices are
pairwise adjacent:

```math
\forall\,x,y\in K,\quad x\ne y \implies \{x,y\}\in E.
```

A **maximal clique** cannot be extended to a strictly larger clique:

```math
\nexists\,K' \text{ that is a clique and satisfies } K\subsetneq K'.
```

A **maximum clique** is a clique with the largest cardinality among all
cliques in the graph. Being *maximal* does not necessarily mean being *maximum*.

### 2.3 Perfect elimination ordering (PEO)

Let a vertex ordering be

```math
\alpha=(v_0,v_1,\ldots,v_{n-1}),
```

where $\mathrm{pos}(v)$ is the index of vertex $v$ in the ordering.
The **later neighbors** of a vertex $u$ are

```math
N^+(u)=\{v\in N(u)\mid
\mathrm{pos}(v)>\mathrm{pos}(u)\}.
```

An ordering is a **perfect elimination ordering** when *every* later-neighbor
set is a clique:

```math
\boxed{\forall\,u\in V,\qquad N^+(u)\text{ is a clique}.}
```

**Foundational theorem:**

```math
G\text{ chordal}\iff G\text{ has a PEO}
```

### 2.4 Why a PEO is useful

For every vertex $u$, form a **candidate clique**

```math
\boxed{C_u=\{u\}\cup N^+(u).}
```

The PEO ensures that all vertices in $N^+(u)$ are pairwise adjacent;
by definition, each is also adjacent to $u$. Therefore $C_u$ is a clique.

**Core connection:** A PEO lets us construct candidate cliques
and then eliminate those that are contained in larger candidates.

## 3. Maximum Cardinality Search (MCS)

### 3.1 Algorithmic idea

For every unnumbered vertex $v$, Maximum Cardinality Search maintains
its **weight**: the number of neighbors already numbered by the algorithm.

```math
w(v)=|\{u\in N(v):u\text{ is already numbered}\}|.
```

Initially, $w(v)=0$ for all vertices. MCS builds the ordering **from right
to left**:

1. Select an unnumbered vertex having the largest current weight.
2. Put it in the next free position, starting at $n-1$ and moving to $0$.
3. Mark it as numbered.
4. For each unnumbered neighbor $u$ of that vertex, increase its weight by one.

The weight update is:

```math
w(u)\gets w(u)+1
```

MCS is defined for any undirected graph; however, its output is guaranteed
to be a PEO **only when the input graph is chordal**.

### 3.2 Bucket-based implementation

| C++ structure | Mathematical meaning / role |
| --- | --- |
| weight[v] | The weight $w(v)$ |
| numbered[v] | Whether vertex $v$ has already been selected |
| buckets[w] | Linked list of unnumbered vertices with weight $w$ |
| locations[v] | Iterator to $v$ inside its current bucket |
| max_weight | Maximum bucket weight currently considered |
| ordering[i] | Vertex placed at index $i$ of the output PEO |

The iterator array permits $O(1)$ removal and reinsertion when a vertex
weight increases. Stable iterators from C++ linked lists and deterministic
front insertions preserve the sample ordering.

### 3.3 Pseudocode

Pseudocode remains plain text because these are program operations,
**not** equations:

~~~text
MCS(G):
    for v in V:
        weight[v] = 0
        numbered[v] = false
        push v into buckets[0]
        remember locations[v]
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
~~~

### 3.4 Correctness statement and proof overview

**Theorem:** For a chordal graph $G$, MCS produces a PEO $\alpha$.

```math
G\text{ chordal}\ \Longrightarrow\ \mathrm{MCS}(G)\text{ produces a PEO}
```

A contradiction-proof overview, attributed to Vandenberghe and Andersen,
proceeds as follows:

1. Assume the produced ordering is not a PEO. Consider a chordless path
   $P=(v_0,v_1,\ldots,v_k)$, $k\ge 2$, such that its interior vertices
   appear earlier in the ordering than its endpoints.
2. Choose such a path to maximize the smaller endpoint position.
   Label the endpoints so that
   $\mathrm{pos}(v_0)>\mathrm{pos}(v_k)>\mathrm{pos}(v_1)$.

3. When MCS selected $v_k$, it had weight at least that of $v_1$.
   But the already-numbered $v_0$ contributed to the weight of $v_1$
   and not to the weight of $v_k$. Hence $v_k$ must have an additional
   already-numbered neighbor $z$ not adjacent to $v_1$.
4. That extra neighbor can either close a forbidden chordless cycle
   (contradicting chordality), or yield a forbidden path with a strictly
   better endpoint position (contradicting the maximization choice).
5. Both alternatives are impossible; the MCS ordering must be a PEO.

> **Proof status:** This is a readable outline, not an independently checked
> full derivation. The final thesis proof still requires verification of
> the exact quantifiers, ordering convention, and edge cases.

### 3.5 Complexity analysis

Let $n=|V|$ and $m=|E|$.

| Step | Total time |
| --- | --- |
| Initialize vertices and buckets | $O(n)$ |
| Extract each vertex once | $O(n)$ |
| Scan undirected adjacency entries | $2m=O(m)$ |
| Move one vertex between buckets | $O(1)$ per weight update |
| Scan empty buckets | $O(m)$ total (amortized) |

Every increase of the max-weight tracker is caused by an edge scan,
and total decreases cannot exceed accumulated increases. Thus the
overall runtime of the MCS ordering computation is

```math
\boxed{T_{\mathrm{MCS}}(n,m)=O(n+m).}
```

Space, including the stored graph, is $O(n+m)$.

## 4. Verifying a perfect elimination ordering

The modular implementation verifies the PEO condition before any
maximal-clique extraction.

For a vertex $u$ with nonempty $N^+(u)$, let $p(u)$ be its
**earliest later neighbor**, meaning the vertex with smallest later
position:

```math
p(u)=
\underset{v\in N^+(u)}{\mathrm{arg\,min}}
\ \mathrm{pos}(v).
```

For every other later neighbor $v$, the verifier requires an edge from
$p(u)$ to $v$:

```math
\boxed{
\forall\,u\in V,\quad
\forall\,v\in N^+(u)\setminus\{p(u)\},
\qquad \{p(u),v\}\in E.
}
```

When $N^+(u)=\varnothing$, the condition is vacuous. Testing it for all
vertices is sufficient for the PEO property (by induction on the ordering).

The current implementation performs this verification **deterministically**:
first compute the earliest later neighbor $p(u)$ for each vertex $u$.
Group vertices by the value of $p(u)$. For each possible parent $p$,
mark all neighbors of $p$ **once**, then scan the adjacency lists of
every child $u$ assigned to $p$ and verify that every later neighbor of
$u$ other than $p$ has been marked. The marker is the parent vertex ID,
so there is no need to clear the entire marker array between groups.

Each adjacency list is scanned only a constant number of times across
all groups; no hash-table membership query is required. This gives:

```math
T_{\mathrm{PEO\ verification}}(n,m)=O(n+m)
```

This is a **worst-case** bound for the PEO verification computation,
and its additional memory is $O(n+m)$ for grouped child lists,
positions, parents, and the marking array.

Invalid-length orderings, repeated vertices, and orderings that fail the
PEO condition are rejected. If the MCS-generated ordering fails, the
CLI reports a non-chordal input and exits with status 2.

**Complexity note:** PEO verification and MCS both now run in
deterministic worst-case $O(n+m)$ time on an input graph already
represented by adjacency lists. The input parser's duplicate-edge
hash table is separate from this algorithmic bound.

## 5. Maximal clique extraction

### 5.1 Candidate cliques

Let $\alpha$ be a PEO and use the notation

```math
\mathrm{Madj}(u)=N^+(u).
```

The **candidate clique** of vertex $u$ is

```math
\boxed{C_u=\{u\}\cup\mathrm{Madj}(u).}
```

The later-neighbor set is a clique under a PEO, and $u$ is adjacent to
every vertex in that set. Hence every $C_u$ is a clique.

**Lemma: Every maximal clique occurs among the candidate cliques.**

**Proof:** Take an arbitrary maximal clique $K$. Let $u$ be the
earliest vertex of $K$ in the PEO. Every other vertex of $K$ is later
than $u$ and adjacent to it. Thus

```math
K\setminus\{u\}\subseteq\mathrm{Madj}(u)
\quad\Longrightarrow\quad
K\subseteq C_u.
```

Since $C_u$ is a clique and $K$ is maximal, this inclusion cannot be
strict. Therefore

```math
\boxed{K=C_u.}
```

### 5.2 Parent vertex and the key inclusion

When $\mathrm{Madj}(u)\ne\varnothing$, define the **parent**
$p(u)$ as the *earliest* later neighbor of $u$:

```math
p(u)=\underset{v\in\mathrm{Madj}(u)}
{\mathrm{arg\,min}}\ \mathrm{pos}(v).
```

When the later-neighbor set is empty, the code uses **-1 as a sentinel**
rather than a vertex ID.

Fix a vertex $u$ with parent $p=p(u)$. Because
$\mathrm{Madj}(u)$ is a clique, all other later neighbors of $u$
are adjacent to $p$. As $p$ is the earliest among them, the remaining
vertices also occur later than $p$. This proves the important inclusion (1):

```math
\boxed{
\mathrm{Madj}(u)\setminus\{p\}
\subseteq\mathrm{Madj}(p).
}
```

### 5.3 Parent–child subsumption condition

The implemented maximality check compares the sizes of the two sets (2):

```math
\boxed{
|\mathrm{Madj}(u)|-1=
|\mathrm{Madj}(p)|.
}
```

**Why does this test work for the given parent–child pair?**

From inclusion (1), the left-hand set is *already contained* in the
right-hand set. When their sizes are equal, the two sets are identical:

```math
\mathrm{Madj}(u)\setminus\{p\}
=\mathrm{Madj}(p).
```

Consequently, we obtain the strict candidate containment in (3):

The candidate for $p$ equals the later-neighbor set of $u$:

```math
C_p=\{p\}\cup\mathrm{Madj}(p)=\mathrm{Madj}(u)
```

Adding $u$ produces a strictly larger clique:

```math
C_p\subsetneq \{u\}\cup\mathrm{Madj}(u)=C_u
```

Thus $C_p$ is **not maximal** when the size equality holds for some
child $u$.

Conversely, if $C_p\subsetneq C_u$ for this same parent–child pair,
the set containment forces the corresponding later-neighbor sets to
be equal, which gives the size equality again.

**Pairwise equivalence (for $p=p(u)$):**

```math
\boxed{
C_p\subsetneq C_u
\iff
|\mathrm{Madj}(u)|-1=|\mathrm{Madj}(p)|.
}
```

> **Open proof obligation:** This parent–child *pairwise* equivalence does
> not on its own establish that *every* non-maximal candidate is detected
> by some tested parent–child relationship. A rigorous global-completeness
> argument is still needed. Exhaustive oracle tests provide evidence but
> do not replace a proof.

### 5.4 Extraction algorithm

Algorithmic pseudocode uses a code block; the mathematical inclusion
and cardinality relations above are written in typeset LaTeX.

~~~text
EXTRACT_MAXIMAL_CLIQUES(G, peo):
    verify peo is valid for G
    position = inverse permutation of peo

    for each vertex u:
        Madj(u) = neighbors appearing later than u
        parent[u] = earliest vertex of Madj(u), or -1
        later_degree[u] = size of Madj(u)

    maximal[u] = true for every vertex
    for each vertex u:
        p = parent[u]
        if p != -1 and later_degree[u] - 1 == later_degree[p]:
            maximal[p] = false

    result = []
    for u in peo order:
        if maximal[u]:
            result.append({u} union Madj(u))
    return result
~~~

### 5.5 Complexity and output size

Positions, later-neighbor lists, parents, and sizes are computed with
$O(n+m)$ work. The parent-subsumption checks take $O(n)$.

Let $\mathcal{K}_{\max}$ be the set of reported maximal cliques.
The number of **vertex occurrences** in the materialized output is

```math
\boxed{S=\sum_{K\in\mathcal{K}_{\max}}|K|.}
```

Constructing the output requires $O(S)$ time. With the deterministic
linear PEO check, the total running time of extraction is

```math
\boxed{
T_{\mathrm{extract}}(n,m,S)=O(n+m+S).
}
```

Space including graph and materialized output is $O(n+m+S)$.
The later-neighbor lists themselves occupy $O(n+m)$ space.

Explicitly including $S$ clarifies the cost of output construction.

## 6. Seven-vertex example and dry run

The graph in the sample data file contains seven vertices and ten edges.

```math
V=\{0,1,2,3,4,5,6\}
```

The **10 undirected edges** (written as endpoint pairs) are:

- $(0,1)$, $(1,3)$, $(0,2)$, $(2,3)$, $(2,4)$
- $(2,5)$, $(4,5)$, $(3,6)$, $(0,3)$, $(1,2)$

The complete input is also available in `data/example_7_vertices.txt`.

MCS selects vertices in this order with the chosen bucket tie-breaking:

```math
6\ \longrightarrow\ 3\ \longrightarrow\ 0\ \longrightarrow\
2\ \longrightarrow\ 1\ \longrightarrow\ 5\ \longrightarrow\ 4.
```

Because MCS fills the ordering from **right to left**, the resulting
perfect elimination ordering is the **reverse** of the selection sequence:

```math
\boxed{\alpha=(4,5,1,2,0,3,6).}
```

The extracted maximal cliques are

- **Clique 1:** $K_1=\{0,1,2,3\}$
- **Clique 2:** $K_2=\{2,4,5\}$
- **Clique 3:** $K_3=\{3,6\}$

Each is a clique, none is a proper subset of another, and the
independent oracle confirms that there are no further maximal cliques
in this example. Vertex order within a clique has no mathematical
significance.

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
| Direct clique tree | `include/mtp/clique_tree_blair_peyton.hpp` | `src/clique_tree_blair_peyton.cpp` | $O(n+m)$ clique-tree preprocessing without full $W_G$ |
| CLI | - | `src/main.cpp` | Read graph files and use the direct constructor |
| Tests | - | `tests/test_algorithms.cpp` | Independent oracle checks |

The public algorithm functions accept a read-only `Graph` reference. This
keeps the algorithm stages separate from I/O. Future modules should follow
the same pattern instead of expanding a monolithic class.

### 7.3 Input contract and error handling

A graph file begins with two integers $n$ and $m$, followed by $m$ undirected edges represented as pairs of zero-based vertex IDs.
The vertices are numbered from zero. The loader rejects invalid headers, missing
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
and pull requests once published to GitHub. Re-run CI after each change; the workflow runs unit tests and the Markdown math checker.

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
| Exhaustive graph coverage | Every simple graph with $n=1,\ldots,6$ | PASS |
| Chordal instances within exhaustive coverage | 19,048 chordal graphs | PASS |
| Remaining non-chordal instances | Compared to chordality oracle | Correctly rejected |
| Random chordal coverage | 600 seeded instances, 7–12 vertices | PASS |
| Edge cases | Empty, isolated and disconnected graph; duplicate edge behavior; self-loop rejection; 4-cycle | PASS |

For a simple undirected graph on $n$ labeled vertices, each of the
$\binom{n}{2}$ possible edges can be present or absent. Hence, there
are

```math
2^{\binom{n}{2}}
```

such graphs. Exhaustively testing all graph sizes $n=1,2,\ldots,6$
therefore requires

```math
\sum_{n=1}^{6}2^{\binom{n}{2}}
```

Evaluating this expression gives:

```math
2^0+2^1+2^3+2^6+2^{10}+2^{15}
```

Hence, the total number of graph inputs is **33,867**.

graph inputs.

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

The implementation roadmap follows the paper's **linear-time
preprocessing** goal, and distinguishes production work from reference
implementations:

```text
[Chordal foundations + MCS / PEO] ----- COMPLETE
                 |
                 v
[Maximal cliques] --------------------- COMPLETE
                 |
                 v
[Direct clique-tree construction] ----- COMPLETE: O(n+m)
                 |
                 v
[Ibarra Section 3 edge queries] ------- COMPLETE (A1)
                 |
                 v
[Ibarra Section 3 edge updates] ------- COMPLETE (A1)
                 |
                 v
[Section 4 O(n) implementation] ------- COMPLETE (A1)
                 |
                 v
[Section 6 fast Delete-Query] --------- COMPLETE (A1+)
                 |
                 v
[Section 5 worst-case dynamic tree] --- NOT IMPLEMENTED (A2/A2+)

[Section 8 split graph + 4 numbers] --- COMPLETE

Explicit W_G + Kruskal = test baseline ONLY.
```

### 9.1 Reference-only: explicit weighted clique intersection graph

**Primary source:** Ibarra (2008), Section 2, paper page **40:4**.

Let the family of maximal cliques of the chordal graph be

```math
\mathcal{K}_G=\{K_0,K_1,\ldots,K_{k-1}\}.
```

The weighted clique intersection graph **W_G** has one node per maximal
clique. Two *different* clique nodes are connected **if and only if**
their underlying cliques intersect:

```math
\{i,j\}\in E(W_G)\iff K_i\cap K_j\ne\varnothing.
```

Every such edge has an integer weight equal to the intersection size:

```math
w(i,j)=|K_i\cap K_j|.
```

**Do not confuse** the original vertices of G with the nodes of W_G.
Nodes K0, K1, and K2 in the CLI refer to maximal cliques, not graph vertices.

**Non-production baseline:** The function
<code>build_weighted_clique_graph(...)</code> in
<code>src/clique_graph.cpp</code> receives the maximal cliques from the
previous extraction stage, sorts each clique's vertex list, and examines
every pair. For each pair it computes intersection size using a
two-pointer scan through the sorted lists. It creates an edge only when
the intersection size is positive. It also rejects empty cliques,
negative vertex IDs, and duplicate vertex IDs within a clique.

**Correctness argument:** On a given pair of sorted clique member
lists, a two-pointer intersection scan counts each common vertex once
and does not count vertices absent from either list. Consequently, for
each pair (i,j) it returns exactly the size of the set intersection.
The algorithm retains precisely the pairs with positive size, matching
the edge definition above. It introduces no extra positive-weight edges.

**Time complexity:** Let S be the total number of clique-member
occurrences and let k be the number of maximal cliques. Sorting requires
a total of at most O(S log n) time. All pairwise intersections take

```math
O((k-1)S)
```

time in total, because each clique appears in exactly k-1 pairs.
The algorithm also examines O(k²) candidate pairs. Therefore an
appropriate upper bound is **O(S log n + kS + k²)**, so this module
does **not** meet Ibarra's $O(n+m)$ preprocessing requirement. The
full weighted clique graph can contain $O(k^2)$ edges. This module
is excluded from the production CLI and isolated in a test-only
CMake library.

### 9.2 Reference-only: Kruskal-based clique tree

**Primary source:** Ibarra (2008), Section 2, paper pages **40:4–40:6**.

For a connected chordal graph, a clique tree T is a tree whose nodes
are exactly the maximal cliques and which satisfies the **induced
subtree property**:

> For every vertex v of G, the clique nodes containing v must induce
> a connected subtree of T.

An equivalent form, called the **clique intersection property**, says
that for any maximal cliques Ki and Kj, every clique on their unique
path in the tree contains the intersection of Ki and Kj.

The paper states a key characterization for **connected** graphs:

> A spanning tree of the weighted clique intersection graph is a
> clique tree if and only if it has maximum total edge weight.

**Reference construction:** Implemented in <code>src/clique_tree.cpp</code>.
It is retained for independent testing and **is not used by the CLI**.

1. Sort the positive-weight edges of W_G by descending weight.
2. Run Kruskal with disjoint-set union, adding edges that join two
   different sets. This constructs a **maximum-weight spanning forest**.
3. If G was disconnected, join the remaining clique-tree components
   by arbitrary **dummy edges of weight zero**, as described by Ibarra,
   Section 2, paper page 40:6.

The dummy edges are not positive-weight edges of the original W_G.
They are added **only** to connect strict clique trees from different
connected components of the underlying graph. The resulting T is
a single tree (for a nonempty graph), with zero-weight dummy edges
where necessary.

**Why construction is correct:** Kruskal's maximum-spanning-tree
greedy rule produces a maximum-weight spanning tree in each connected
component of W_G. The characterization in Section 2 then guarantees
the induced subtree property for the corresponding chordal component.
Dummy links between disconnected components do not create extra
cliques containing any vertex; therefore the induced subtree
condition remains valid for the whole graph.

**Verification function:** <code>verify_clique_tree(...)</code> checks
all of the following independently of the Kruskal selection sequence:

1. There are exactly k-1 edges when k>0; there are no cycles and
   all k clique nodes are connected.
2. Each weight equals the actual number of common clique vertices.
3. Every dummy edge has weight zero and links different components
   of the original positive-weight W_G.
4. For each original vertex v, a BFS restricted to clique nodes
   containing v reaches **all** clique nodes containing v.

**Time complexity:** Let q be the number of positive-weight edges of W_G.
Sorting edges costs O(q log q), and disjoint-set operations cost
O((q+k) alpha(k)). The algorithm copies the clique member lists,
adding O(S) time and space. Since q is at most O(k²), this is a simple
static construction rather than an implementation of the paper's
fully dynamic data structures.

### 9.2.1 Worked example

For the 7-vertex input in Section 6, the maximal cliques (the CLI
numbers them in extraction order) are:

- **K0** = {2,4,5}
- **K1** = {0,1,2,3}
- **K2** = {3,6}

The nonempty intersections are:

- K0 intersect K1 = {2}, so weight = 1.
- K1 intersect K2 = {3}, so weight = 1.
- K0 intersect K2 is empty, so **no edge** is created in W_G.

Thus W_G contains two weighted edges, K0--K1 and K1--K2, both
having weight 1. The graph itself is already a tree, so the
maximum-weight spanning tree is the same two-edge structure.

**Disconnected example:** If the input has the two components
{0,1} and {2,3}, then its two maximal cliques have intersection
size zero. The original W_G has **no edges**; the extended clique
tree has one **dummy edge** of weight zero connecting these cliques.

### 9.2.2 Testing and scope

The new source <code>tests/test_clique_tree.cpp</code> verifies:

- The example clique graph has exactly 2 intersection edges of
  weight 1, and the resulting tree satisfies its invariants.
- Disconnected cases require the expected zero-weight dummy links.
- Exhaustive chordal graphs for n=1..6: all **19,048** cases satisfy
  the clique-tree invariants and the expected clique intersection
  graph properties.
- For all chordal inputs with n<=5, independently enumerating
  labeled spanning trees by Prüfer sequences confirms that the
  constructed tree has **maximum total intersection weight**.
- 600 additional seeded chordal graphs on n=7..12 pass.
- Invalid clique data and deliberately incorrect dummy flags are
  rejected in targeted tests.

These tests do **not** prove performance for large input sizes,
and do **not** exercise incremental dynamic edge operations.

### 9.2.3 Production: published Blair–Peyton expanded MCS

**Reference:** Blair and Peyton (1993), Section 4.2.2, Figure 4.2,
Theorem 4.3. The former elimination-forest contraction prototype
has been deleted because its correspondence to a published algorithm
was unverified.

The production function `build_clique_tree_blair_peyton()` replays the
**MCS-derived** PEO right to left. It counts already-numbered neighbors,
starts a new maximal clique whenever the count does not exceed the
previous count, and connects that clique to the clique containing its
lowest-labeled separator vertex. This follows Figure 4.2's decision
rules. Disconnected components are joined by dummy zero-weight links
as prescribed by Ibarra (2008), Section 2.

The construction scans each adjacency list a constant number of times.
Clique member insertions total `O(n+m)`; therefore its worst-case
running time is **O(n+m)** for a represented graph with MCS ordering.
Correctness is justified by Blair–Peyton's theorem and checked by
independent exhaustive/random regression tests (Section 13.1).
No pairwise construction of `W_G` occurs in the production path.

### 9.3 Remaining: Section 5 dynamic-tree optimization and Section 7 comparisons

First implementation (Sections 3–4) is complete. The remaining faster
variants must be implemented from the paper, with their exact supporting
data structures. For each new optimized operation:

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

### 9.5 Mandatory source-to-code traceability policy

**Project rule (2026-11-18): No original or invented algorithmic methods.**
The objective is to implement Ibarra (2008) using the methods and
asymptotic bounds actually supported by published research.

An algorithm may be accepted only if each nontrivial technique is
supported either by an exact location in Ibarra (2008) or by a
**verified published primary reference**. A valid source must provide
enough detail to audit the implementation (pseudocode/algorithm,
lemma/proof, or an unambiguous stated method), together with its
preconditions and complexity. Do not infer source fidelity solely from
successful tests or matching a time bound. Mathematical explanations
of published algorithms are welcome, but they must not be passed off
as newly invented implementation methods.

**Verified sources for initial clique-tree construction:**

1. **Blair and Peyton (1993), Section 4.2, Figure 4.2**, in
   *An Introduction to Chordal Graphs and Clique Trees*
   (Springer chapter; also an accessible Oak Ridge report).
   Their expanded **Maximum Cardinality Search** detects maximal cliques
   and produces clique-tree edges as the ordering is constructed.
   Section 4.2 proves its connection to Prim's maximum-weight spanning
   tree algorithm. Original report:
   https://people.math.binghamton.edu/zaslav/Oldcourses/580.S13/blair-peyton.chordal-graphs-clique-trees.ornl1992.pdf

2. **Habib, McConnell, Paul and Viennot (2000), Section 3.1, Algorithm 4**,
   *Lex-BFS and Partition Refinement, with Applications to Transitive
   Orientation, Interval Graph Recognition and Consecutive Ones Testing*,
   *Theoretical Computer Science* 234, 59–84.
   A PEO/right-neighbor/parent-based clique-tree algorithm is explicitly
   presented with an **O(n+m)** running-time analysis and induced-subtree
   correctness proof. DOI: 10.1016/S0304-3975(97)00241-7.
   Source: https://doi.org/10.1016/S0304-3975(97)00241-7

3. **Independent confirmation:** Chandran, Ibarra, Ruskey and Sawada
   (2003), *Generating and Characterizing the Perfect Elimination
   Orderings of a Chordal Graph*, *Theoretical Computer Science* 307,
   303–317, Section 4.1, specifically identifies Blair and Peyton's
   **O(n+m)** clique-tree construction.
   DOI: 10.1016/S0304-3975(03)00221-4.

**Source audit closed (2026-11-19):** The provisional
elimination-forest constructor was removed. Production now uses
`build_clique_tree_blair_peyton()`, mapped to published Figure 4.2
in Section 13.1.

**Acceptance record required for every module:**
paper and bibliographic identity; exact section/algorithm/lemma;
input assumptions (including connected vs. disconnected graphs);
source pseudocode-to-function correspondence; proof obligations;
worst-case time and space bounds; tests and benchmark results; and
explicitly identified implementation departures. Unapproved modules
cannot be marked paper-faithful.

## 10. Open questions and proof obligations

| Item | Why it matters | Status |
| --- | --- | --- |
| Confirm Ibarra's exact clique-graph edge definition | Verified against Ibarra Section 2 | COMPLETE |
| Confirm required clique-tree representation | Verified static clique-tree properties in Section 2 | COMPLETE |
| Validate complete parent-rule filtering proof | A proof sketch is included in Section 9.2.3; check against cited primary literature | IN REVIEW |
| Replace unsupported production clique-tree method | Blair–Peyton Fig. 4.2 integrated and source mapped | COMPLETE |
| Validate paper's stated complexity bounds from primary text | Table in Section 7 checked; Section 5 remains pending | PARTIAL |
| Implement Sections 3–4 dynamic queries | Theorems 4–6 and Section 4 | COMPLETE — TESTED |
| Implement Section 3 incremental updates and compare against rebuilding | Clique tree split, contract, swap | COMPLETE — TESTED |
| Benchmark runtime/scaling for all variants | Experimental evaluation | OPEN |

## 11. Development log

| Date | Milestone | Evidence |
| --- | --- | --- |
| 2026-11-18 | Refactored code into separate C++17 graph, MCS, and clique modules | `src/`, `include/` |
| 2026-11-18 | Added PEO verification, CLI input handling, tests, CMake, CI configuration | Source and test files |
| 2026-11-18 | Exhaustive verification for $n\le6$ and 600 random chordal cases | `tests/test_algorithms.cpp` |
| 2026-11-18 | Added weighted clique graph and static clique tree | `src/clique_graph.cpp`, `src/clique_tree.cpp` |
| 2026-11-18 | Verified 19,048 chordal cases, brute-force maximum tree checks, and 600 random cases | `tests/test_clique_tree.cpp` |
| 2026-11-18 | Replaced production clique-graph enumeration with direct linear clique-tree construction | `src/clique_tree_blair_peyton.cpp`, `src/main.cpp` |
| 2026-11-18 | Replaced expected-time hash PEO validation with worst-case linear grouped marking | `src/mcs.cpp` |
| 2026-11-18 | Added 19,048 exhaustive, 800 randomized, and large-graph tests of direct clique tree | `tests/test_linear_clique_tree.cpp` |
| 2026-11-19 | Replaced provisional preprocessing with Blair–Peyton Fig. 4.2 | `src/clique_tree_blair_peyton.cpp` |
| 2026-11-19 | Implemented Ibarra first dynamic chordal updates and oracle tests | `src/dynamic_ibarra.cpp`, `tests/test_ibarra_dynamic.cpp` |
| 2026-11-19 | Implemented Ibarra O(1) split recognition and updates | `src/dynamic_split_ibarra.cpp`, `tests/test_ibarra_split.cpp` |
| 2026-11-19 | Extended Section 8 to four optimization numbers via complement degrees | `tests/test_ibarra_split_stats.cpp` |
| 2026-11-19 | Implemented Ibarra Section 6 fast delete index and its update hooks | `src/fast_delete_ibarra.cpp`, `tests/test_ibarra_fast.cpp` |
| Pending | Ibarra Section 5 worst-case dynamic-tree implementation and combined A2+ | Not claimed as completed |

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

7. Habib, M., McConnell, R. M., Paul, C., and Viennot, L. (2000).
   *Lex-BFS and Partition Refinement, with Applications to Transitive
   Orientation, Interval Graph Recognition and Consecutive Ones Testing*.
   Theoretical Computer Science, 234(1–2), 59–84.
   DOI: 10.1016/S0304-3975(97)00241-7.
8. Chandran, L. S., Ibarra, L., Ruskey, F., and Sawada, J. (2003).
   *Generating and Characterizing the Perfect Elimination Orderings of a
   Chordal Graph*. Theoretical Computer Science, 307, 303–317.
   DOI: 10.1016/S0304-3975(03)00221-4.

## 13. Paper-faithful implementation milestones — November 19, 2026

This section overrides the older *baseline* status in Sections 7–9. In
particular, an explicitly constructed weighted clique graph and Kruskal are
not part of the production preprocessing path. They remain as test oracles.

### 13.1 Published MCS-to-clique-tree preprocessing

**Primary reference:** Blair and Peyton (1993), *An Introduction to
Chordal Graphs and Clique Trees*, **Section 4.2.2, Figure 4.2**;
especially Lemmas 7–10 and Theorem 4.3. The authors explicitly connect
their expanded MCS to Prim's maximum-weight spanning-tree algorithm.
Ibarra (2008), Section 2, cites earlier linear-time preprocessing methods.

**Source-to-code mapping:**

| Figure 4.2 operation | C++ implementation |
| --- | --- |
| Select a maximum-cardinality numbered vertex | `maximum_cardinality_search(G)` |
| Compute `new_card` as numbered-neighbor count | `new_cardinality` in `src/clique_tree_blair_peyton.cpp` |
| Begin new clique when new cardinality is at most previous | `if (new_cardinality <= previous_cardinality)` |
| Start clique with numbered neighbors | Add each later neighbor of chosen vertex |
| For nonempty separator, choose its smallest PEO index | `lowest_label_neighbor` |
| Link new clique to clique assigned to selected separator vertex | `clique_of_vertex[minimum_labeled_neighbor]` |
| Assign current vertex to current clique and add it | `clique_of[v]=active_clique` and `push_back(u)` |

The MCS selection stage is already carried out by
`maximum_cardinality_search`; the construction replays Figure 4.2's
MCS ordering and clique/edge decisions in reverse PEO order.
For disconnected graphs, a zero-cardinality new clique starts a new
strict clique tree, joined by dummy edge weight zero in accordance
with **Ibarra (2008), Section 2**.

**Complexity:** Numbered-neighbor scans sum to `O(n+m)` across
all vertices; creation of all clique-member lists also totals
`O(n+m)`. There is no enumeration of all edges of `W_G`.
The constructor uses `O(n+m)` time and space for an adjacency-list
input and a valid MCS-derived PEO, including PEO verification.

**Verification:** `tests/test_linear_clique_tree.cpp` now exercises
the Blair–Peyton constructor on 19,048 chordal nonempty graphs
with at most six vertices, 800 randomized chordal inputs,
and larger star/complete graphs. It compares maximal cliques,
running-intersection connectivity and intersection edge weights
with independent reference computations. Source: Blair–Peyton
Figure 4.2 is the correctness basis; tests detect implementation defects.

### 13.2 Ibarra first dynamic implementation: complete source mapping

**Primary reference:** Ibarra (2008), Sections **3.1–3.4**,
Theorems **4–6**, and **Section 4**.
**Code:** `include/mtp/dynamic_ibarra.hpp`,
`src/dynamic_ibarra.cpp`; CLI: `src/dynamic_main.cpp`.

The class stores each active maximal clique with a characteristic
vector and maintains a connected clique-tree representation, using
weight-zero dummy tree links across disconnected components.

#### Delete-Query(u,v) — Ibarra Theorem 4

For an existing edge, deletion preserves chordality exactly when
the edge appears in a **unique maximal clique**.

```math
G-\{u,v\}\text{ chordal}\iff
|\{K\in\mathcal{K}_G: u,v\in K\}|=1
```

The implementation scans the at-most-n clique nodes, testing the two
vertex-membership bits in constant time per node. The query changes
neither the graph nor the tree: **O(n) worst case**.

#### Delete(u,v) — Ibarra Section 3.2

If the query fails, reject without changing the representation.
Otherwise, find the unique clique `K_x` containing `u,v`, form
`K_x\setminus\{v\}` and `K_x\setminus\{u\}`, and replace `x`
by two new clique nodes joined by a weight-`|K_x|-2` edge.
Reattach each former neighbor to the appropriate new node depending
on whether its clique contains `u`, `v`, or neither.
The former neighbors with weight `|K_x|-1` determine whether
either new candidate is contained in an existing maximal clique.
Contract the corresponding tree edge(s) when such containment holds.

Tree edges are stored in linked adjacency lists with bidirectional
iterators for **constant-time individual link/cut**. Node membership
tests are constant-time characteristic-vector accesses. At most
`O(n)` adjacency edges and `O(n)` membership entries change.
Accepted deletion: **O(n) worst case** (excluding optional
snapshot/verification output).

#### Insert-Query(u,v) — Ibarra Theorems 5–6

For a nonedge, find the closest nodes `x` in the clique subtree
containing `u` and `y` in the subtree containing `v`.
If `x,y` are adjacent, insertion is allowed. Otherwise the
necessary and sufficient criterion compares the minimum edge
weight on the `x-y` tree path with `|K_x\cap K_y|`.

```math
\min_{e\in P_T(x,y)}w(e)=|K_x\cap K_y|
```

The first implementation finds the two subtrees' closest
boundary pair by tree traversal, inspects their connecting
path, and computes the endpoint intersection by scanning
characteristic vectors: **O(n) worst case**. It is nonmutating.

#### Insert(u,v) — Ibarra Section 3.4

If necessary, replace a minimum-weight tree edge on the
`x-y` path with `x-y` (Theorem 6); then replace that edge
with a new node `z` whose clique is:

```math
K_z=(K_x\cap K_y)\cup\{u,v\}
```

Connect `z` to `x` and `y`, each with weight
`|K_x\cap K_y|+1`. Contract the old `x` and/or `y`
if their clique size equals `|K_x\cap K_y|+1`.
This is exactly the subsumption test in Section 3.4.
Accepted insertion: **O(n) worst case**, excluding debug output.

**Validation:** `tests/test_ibarra_dynamic.cpp` checks
each query against independently changing a copy of the graph and
running MCS/PEO validation. For every accepted edge update, it
reextracts maximal cliques independently, checks separator weights,
counts nodes/edges, and checks the induced-subtree property.
It tests all initial chordal graphs of size at most five and
random longer sequences on 200 chordal graphs of up to twelve vertices.
Testing is evidence, not proof; the referenced theorems justify the
algorithmic transformation.

**Important preprocessing limitation:** Construction of the initial
Blair–Peyton clique tree is linear, but the first dynamic class also
allocates a length-n characteristic vector for **each** maximal clique.
For k maximal cliques, its concrete initialization and storage cost
contains `Theta(nk)`. On sparse stars, `k=Theta(n)` and this costs
quadratic time/space. This is a real difference between the current
C++ implementation's concrete initialization and the paper's
`O(n+m)` preprocessing entry in Section 7. It **must not** be
reported as linear end-to-end preprocessing without resolving
the representation discrepancy.

### 13.3 Ibarra Section 8: dynamic split graphs

**Primary reference:** Ibarra (2008), Section **8**, Theorem **8**
(Hammer–Simeone characterization).
**Code:** `include/mtp/dynamic_split_ibarra.hpp`,
`src/dynamic_split_ibarra.cpp`.

Let the sorted degree sequence be `d_1>=...>=d_n`.
Define `j` as the largest index with `d_j>=j-1`.
Ibarra's split characterization is:

```math
\sum_{i=1}^{j}d_i=j(j-1)+\sum_{i=j+1}^{n}d_i
```

The implementation maintains a doubly linked list of degree entries
in descending order, pointer ranges for each degree value, the
partition boundary `j`, and the two partition sums.
On a valid insertion it increments the **first** element(s) of
the two affected degree classes. On a valid deletion it decrements
the **last** element(s). The sorted order is preserved.
At most one degree entry crosses the partition boundary per update,
so the degree sequence and both sums are maintained in **O(1)**.
`is_split()` compares the two maintained sums in **O(1)**.
Counting-sort initialization and graph-degree scans are **O(n+m)**.

**Four optimization numbers (Ibarra, Section 8, final paragraph):**
When the current graph is split, Theorem 8 supplies its maximum-clique
size `j`. Because split graphs are perfect, their chromatic number is
also `j`. The **complement of a split graph is itself split**, so
maintaining a second degree-sequence index for the complement supplies
its clique number, which equals the original graph's independence
number. By perfectness, the clique-cover number equals that
independence number.

Implementation: `DynamicSplitStatistics` holds `original_` and
`complement_` degree-sequence indexes. Inserting `(u,v)` into
the original graph increments the original degree entries and
**decrements** the corresponding complement degree entries;
deletion performs the reverse. Each of these paired operations
has `O(1)` worst-case cost in Ibarra's Section 8 representation.
The four queries perform only boundary lookups and return `O(1)`.
They reject numeric queries when the graph is not split.

Independent regression: `tests/test_ibarra_split_stats.cpp` checks
all four values against exhaustive subset enumeration for random
valid update sequences on graphs with at most seven vertices, including
empty and disconnected cases. This is in addition to the earlier
split-recognition update test.

**Precondition:** The caller must insert only an absent edge and
delete only an existing edge. The degree-only data structure does
not store the full graph and cannot independently reject an
invalid sequence without an additional edge dictionary.
This is stated in the interface and follows the paper's valid-update
model. The independent test oracle stores the graph separately
and checks randomized valid sequences. Theorem 8 and Section 8
are the source of the algorithm and complexity bounds.

### 13.4 Ibarra Section 6: modified-Yellin fast deletion queries

**Primary reference:** Ibarra (2008), Sections 6.1–6.3, especially
**Lemma 7** and the member/overflow/COMMON construction.
**Source files:** `include/mtp/fast_delete_ibarra.hpp`,
`src/fast_delete_ibarra.cpp`, optional integration within
`src/dynamic_ibarra.cpp`. **Tests:** `tests/test_ibarra_fast.cpp`.

For each active maximal clique node `x`, partition its vertex set
into `x.member` and `x.overflow`, ensuring that:

```math
|x.member|\le p,\qquad p=\max(1,\lceil\sqrt m\rceil)
```

Each overflow list is nonempty only when its member list is full.
Maintain a common-count matrix:

```math
COMMON[u,v]=|\{x:u,v\in x.member\}|
```

This counts every clique for which both vertices are in the member
portion. The query scans only cliques with **nonempty overflow lists**
and counts those containing both endpoints with at least one endpoint
in the overflow portion.

The paper proves that the total number of nonisolated clique-member
occurrences is at most `2m` (Lemma 7). Hence the number of
overflow-bearing clique elements is `O(m/p)=O(sqrt(m))`.
Every overflow element is checked in constant time through two
characteristic bits. Therefore the deletion query runs in
**O(sqrt(m)+1) worst-case**. Notably, the fast path does **not**
first call the graph's hash adjacency lookup; a nonedge has
zero common maximal cliques and is correctly rejected directly.

**Maintenance mapping:** When a chordal edge update creates or
removes one or two maximal cliques, the dynamic implementation
notifies the index before the old node's representation is changed
and after the new node is created. Adding/removing a whole element
updates the symmetric COMMON counts for all member pairs
and the global linked list of active overflow elements.
After the edge count changes, `rebalance(m)` moves members
between the two portions to restore the threshold invariants.
The paper's Section 6.3 analysis bounds this by **O(m+n) per
graph update**, counting the O(n) underlying first-implementation
tree mutation as well. Updating a member contributes O(p) COMMON
adjustments; there are O(m/p) overflowing or overfull elements.

**Preprocessing and space:** The implementation creates an
n-by-n COMMON matrix and per-clique characteristic membership
vectors, so it is **not** a linear-time preprocessing variant.
The corresponding Ibarra Section 7 A1+ preprocessing bound is
`O(mn+n^2)`; the C++ implementation explicitly allocates these
matrices and lists.

**Validation:** The optional
`DynamicChordalGraph::enable_fast_delete_queries()` activates
the index. The fast regression test runs the same independent
query-oracle and incrementally updated clique-tree checks as the
standard implementation: exhaustive chordal initial graphs
up to five vertices and longer randomized update sequences.
The dynamic CLI supports `--fast-delete` for this mode.

### 13.5 Required variants — November 19 research status

| Ibarra Section 7 version | Code status | Source-method acceptance |
| --- | --- | --- |
| A1 | Implemented and regressions passing | Accepted functionally; preprocessing-bound audit still open |
| A1+ | Implemented and regressions passing | Accepted functionally; index complexity assumptions documented |
| A2 | Sleator–Tarjan research implementation and integration, differential tests passing | **NOT ACCEPTED: Insert complexity and worst-case guarantees unproved** |
| A2+ | A2 research branch with Section 6 index enabled; CLI tests passing | **NOT ACCEPTED: inherits A2 blockers** |
| Section 8 split graphs | Implemented, regression tests passing | No A2 dependency |

The source-selected method for A2 is **Sleator–Tarjan (1983), Section 5**,
using globally biased binary trees (Bent, Sleator and Tarjan, 1985).
No splay or HLD substitute is used in the A2 research class.
That class is isolated on the branch
`research/sleator-tarjan-worst-case`; the former HLD indexed-query
research experiment is **not A2**.

**Acceptance:** Matching functional output is necessary but not sufficient.
Correctness invariants, deterministic worst-case operation accounting,
source-to-code correspondence, initialization, and space must all pass.
See Section 14 for audit findings and a reproducible counterexample
to the current O(n) insertion proof obligation.

### 13.6 Reproducible Section 4 timing experiment

The timing harness in `benchmarks/benchmark_dynamic.cpp` constructs star
graphs with `n=50,100,200,400,800,1600` vertices and repeatedly performs
200 permitted edge insertions and deletions between distinct leaves.
Each insertion creates a triangle; deleting the same leaf–leaf edge
restores the star. Thus no test operation violates chordality.
The benchmark records mean milliseconds for each of the four Ibarra
Section 4 operations and separately times initialization.

**Reproduce with:**

~~~bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DMTP_BUILD_BENCHMARKS=ON
cmake --build build -j
./build/mtp_benchmarks
~~~

**Observed GitHub Actions Ubuntu runner results, November 19, 2026.**
These are single-run descriptive timings, not hardware-independent
guarantees or a formal asymptotic proof:

| n | build (ms) | Insert-Query avg (ms) | Insert avg (ms) | Delete-Query avg (ms) | Delete avg (ms) |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 50 | 0.02175 | 0.00068 | 0.00163 | 0.00007 | 0.00025 |
| 100 | 0.03474 | 0.00122 | 0.00274 | 0.00009 | 0.00040 |
| 200 | 0.07939 | 0.00207 | 0.00473 | 0.00018 | 0.00056 |
| 400 | 0.17384 | 0.00371 | 0.00822 | 0.00045 | 0.00105 |
| 800 | 0.43729 | 0.00641 | 0.01448 | 0.00087 | 0.00194 |
| 1600 | 1.56191 | 0.01312 | 0.02908 | 0.00177 | 0.00393 |

The observed growth of tree initialization is influenced by
per-clique characteristic-vector allocation. The timings do not
establish O(n) worst-case update bounds; those are justified from
the published algorithm and implementation operation counts.
The optional Section 6 index and a fully implemented A2 dynamic tree
still require dedicated comparative experiments.

### 13.7 Worst-case dynamic-tree source and research integration

**Primary source:** Sleator and Tarjan (1983), *A Data Structure for
Dynamic Trees*, JCSS 26(3):362–391, Section 5, **Theorem 8**.
It gives worst-case O(log n) operations under size partitioning
and **globally biased binary trees**. The supporting source is
Bent, Sleator and Tarjan (1985), *Biased Search Trees*,
SIAM J. Computing 14(3), especially globally biased binary trees.

**Implemented on the research branch:**
`src/globally_biased_binary_tree.cpp` (weighted join/split,
rank/global-bias invariants), `src/worst_case_dynamic_tree.cpp`
(expose, splice, slice, conceal, link, cut, evert, min and kth path
operations), and `src/dynamic_ibarra_a2.cpp` (Ibarra Section 5
closest-clique queries, W updates, and mirrored clique-tree changes).

Standalone ST randomized forest tests and A2-versus-A1 differential
tests pass in the Release workflow. This is **not** an independent
formal proof that every helper operation maintains Theorem 8's
precise worst-case prerequisites. The A2 insertion routine
also dispatches A1 clique-tree mutations through the ST forest,
and therefore performs up to a linear number of log-time
primitive calls on a single insertion; see Section 14.

**Status:** source-method implementation exists as a research
candidate; worst-case algorithmic acceptance **BLOCKED**. The
A2/A2+ commands are research-only; production `main`
remains unchanged.

### 13.8 Primary-paper complexity accounting: explicit unresolved question

**Source cross-check:** The authoritative uploaded Ibarra (2008) PDF
contains a potential inconsistency requiring explicit resolution before
claiming full source-level complexity compliance:

- **Section 5.1, printed page 40:13** specifies an `n × n` intersection
  matrix `W` with one entry for every pair of maximal-clique IDs.
  It states `O(n³)` for direct initial pairwise characteristic-vector
  comparisons, and an alternative **`O(mn)`** method using a
  PEO-compatible sequence of legal edge insertions.
  Dynamic updates to `W` then take **`O(n)`**.
- **Section 7, printed page 40:17** states the A2 preprocessing
  complexity as **`O(m+n log n)`**. On sparse connected graphs,
  `m=O(n)`, whereas explicitly initializing a full `n × n`
  matrix takes `Omega(n²)` time/space. The quoted Section 5.1
  `O(mn)` approach also exceeds the table's A2 bound when
  `m=Theta(n)`.

The literal statements are verified by inspecting the PDF pages,
not by trusting imperfect text extraction. This report does not
silently equate these two figures, or label the discrepancy
resolved. Additional published clarification or a paper-supported
implicit/lazy representation would be required to justify the
smaller preprocessing figure alongside a full weight matrix.

**Related warning:** Section 4 specifies length-`n` characteristic
vectors for up to `n` maximal cliques, yet Section 7 states
`O(m+n)` A1 preprocessing. Our concrete eager byte-vector
representation uses `Theta(nk)` initialization and space for
`k` maximal cliques. We report that implementation cost
rather than asserting end-to-end linear initialization.

**Required validation before accepting A2/A2+:** Implement the
published **worst-case** Sleator–Tarjan (1983) global-biased-tree
dynamic forest and the source-specified Section 5.1 intersection
matrix updates; verify `O(log n)` path/reroot/link/cut primitives
and `O(log² n)` insertion queries, then resolve and report
the preprocessing accounting above. The splay-based amortized
link-cut variant is not an acceptable substitution for the
paper's worst-case claim.

### 13.9 Experimental indexed Section 5 queries (not paper A2)

**Reference mapping and strict status.** This implementation adds a
fully tested *research variant* of Ibarra (2008), Sections 5 and 5.1:
fast insertion-query path search and maintained clique-intersection
weights. It must **not** be labeled the paper's A2 or A2+ until the
worst-case logarithmic dynamic tree is implemented as specified.

**Exact published sources:**

- Ibarra (2008), Sections 3.3–3.4 and 5, Theorems 5–6:
  closest clique-subtree nodes x and y, minimum-weight x–y path,
  admissibility condition, and the optional tree-edge swap.
- Ibarra (2008), Section 5.1: maintained clique-intersection
  weight matrix W, O(n) per-update row formulas, and the
  reverse-PEO edge-insertion preprocessing method.
- Sleator and Tarjan (1983), Section 5, Theorem 8:
  size-based heavy/light edges. The code follows the exact
  criterion that an edge to a child is heavy when twice the
  child's subtree size exceeds the parent's subtree size.

**Files:** include/mtp/indexed_dynamic_ibarra.hpp,
src/indexed_dynamic_ibarra.cpp, src/indexed_main.cpp,
tests/test_ibarra_indexed.cpp, benchmarks/benchmark_indexed.cpp.

**Implemented operations:**

1. Maintain one valid maximal-clique witness for each original
   vertex, updating it when affected cliques are replaced.
2. Rebuild a rooted size-based heavy-path partition after a
   permitted insertion or deletion. A static path segment tree
   stores separator weights; chains support kth-node and path
   minimum queries.
3. Find the closest u-containing and v-containing clique nodes
   using two binary searches along the witness-to-witness path.
   The clique-tree connected-subtree property makes membership
   in u a prefix and membership in v a suffix on that path.
4. Test Ibarra's insertion condition: the minimum weight on
   the closest-node path equals the stored intersection W[x,y].
5. For deletion, update the affected W rows using the Section
   5.1 formulas: removing v from Kx subtracts one from its
   overlap with a surviving Kw exactly when v belongs to Kw.
6. For insertion, compute the new Kz weight row using the
   published three cases (Kx replaced, Ky replaced, or both
   survive), and the x-side/y-side components obtained by
   cutting the minimum-weight tree edge.
7. Initialize W by the **paper-specified method**: begin with
   an empty n-vertex chordal graph and insert the target
   edges in reverse PEO order. This preserves chordality
   after each insertion.

**Proved/established cost for this actual implementation:**

| Operation | Analyzed worst-case time |
| --- | --- |
| Heavy-path index rebuilding after an update | O(n) |
| kth node on a tree path | O(log n) |
| Closest u/v clique nodes by binary search | O(log² n) |
| Minimum separator weight on a tree path | O(log² n) |
| Matrix intersection lookup | O(1) |
| Insert-Query | O(log² n) |
| Insert and Delete, including full index rebuild | O(n) each |
| Delete-Query (first version) | O(n) |
| Initialization with W + reverse-PEO edge insertions | O(n² + mn) |

The operation bounds above are for the **whole indexed variant**.
The code does **not** implement the source's O(log n)-worst-case
dynamic-tree link/cut/reroot primitives. Instead it deliberately
rebuilds a static heavy-path index in O(n) after each update.
That is a transparent method deviation, not a claim to have
reproduced Sleator–Tarjan's globally biased binary tree.

**Tests:** The indexed regression suite compares insert and
delete decisions with the first Ibarra implementation on every
vertex pair, performs permitted update sequences from exhaustive
small chordal initial graphs and 240 larger seeded random cases,
and independently verifies updated maximal cliques, tree weights,
and the running-intersection property. GitHub Actions compiles
and executes both the CLI sample and the query benchmark.

**Measured insertion-query scaling (GitHub Actions Ubuntu runner,
November 19, 2026):** A path graph gives an adversarial long
clique-tree path. Each timing averages 2,000 *non-mutating* queries
on the pair of distant endpoint vertices. Construction times are
reported separately, because the source-specified reverse-PEO
edge-insertion initialization is substantially more expensive here.

| n | A1 initialization (ms) | Indexed initialization (ms) | A1 query (ms) | Indexed query (ms) |
| ---: | ---: | ---: | ---: | ---: |
| 100 | 0.0665090 | 1.1412110 | 0.0013268 | 0.0001179 |
| 200 | 0.0738090 | 4.1025190 | 0.0027351 | 0.0001348 |
| 400 | 0.1450650 | 15.9704420 | 0.0058487 | 0.0001531 |
| 800 | 0.3111210 | 61.9226670 | 0.0121334 | 0.0002492 |
| 1600 | 1.0380990 | 278.1260530 | 0.0218950 | 0.0001838 |

Recorded data: benchmarks/results/a1_vs_indexed_path_2026-11-19.csv.
Run: https://github.com/khemsinghbhati/MTP-3rd-sem/actions/runs/44511341613.
These are machine- and input-specific observations, not worst-case
proofs. The experimental indexed variant illustrates the tradeoff
between fast repeated insertion queries and higher initialization
cost; it does not satisfy the source-specific ST dynamic-tree
implementation requirement.

**Research acceptance:** A2 and A2+ remain **INCOMPLETE** until
the exact worst-case dynamic-tree mechanism is implemented and
verified. Matching the top-level query and update time limits
in this alternative does not establish paper-method fidelity.

> Note (20 Nov 2026): the O(n) insertion blocker documented below
> was addressed by the published-source linear-ST construction
> proved in Section 15; original counterexamples are preserved.

## 14. Independent audit of Ibarra A2/A2+ — November 19, 2026

### 14.1 Audit scope, evidence, and verdict

**Scope:** Verify that the actual C++ research implementation reproduces
Ibarra (2008), Sections 3–7, while satisfying the *worst-case*
operation bounds. The audit inspected
`src/dynamic_ibarra_a2.cpp`, `src/dynamic_ibarra.cpp`,
`src/worst_case_dynamic_tree.cpp`,
`src/globally_biased_binary_tree.cpp`, `src/graph.cpp`,
their headers and tests, and the original published sources.
The audit used source-level cost counting plus reproducible
GitHub Actions Release and sanitizer-based tests.

**Primary sources:** Ibarra (2008), Sections 3.2, 3.4, 4, 5, 5.1,
and 7 (the user-supplied paper); Sleator and Tarjan (1983),
Section 5, Theorem 8
(https://www.cs.cmu.edu/~sleator/papers/dynamic-trees.pdf);
Bent, Sleator and Tarjan (1985), globally biased-tree procedures
(https://doi.org/10.1137/0214041).
The findings below distinguish the paper's claims from the
properties established for our code.

**Audit verdict: FUNCTIONALLY PROMISING; INSERT STILL ASYMPTOTICALLY
UNPROVED AND PREPROCESSING TABLE INCONSISTENT WITH STORAGE.**
A2 and A2+ must not be called
fully reproduced or merged into `main`.
The working A1, A1+, Blair–Peyton preprocessing and split-graph
components remain separate.

### 14.2 Source-to-code traceability

| Published operation | Research implementation | Audit finding |
| --- | --- | --- |
| ST globally biased weighted join/split | `globally_biased_binary_tree.cpp` | Rank, ordering, weighted-path and global-bias unit tests pass; formal worst-case validation remains a proof obligation |
| ST Section 5 expose/conceal/splice/slice | `worst_case_dynamic_tree.cpp` | Differential forest link/cut/evert/path queries pass; size-path and rank invariants are tested but worst-case operation proof must be checked line-by-line |
| Ibarra Section 5 nearest cliques on a path | `dynamic_ibarra_a2.cpp::closest` | Binary searches using ST kth-path queries; correct on tested chordal graphs |
| Ibarra Theorem 6 insertion condition | `insert_query`, `W_`, ST path minimum | Differential agreement with A1 on tested graphs; balanced-set lookup now has worst-case O(log n) bound |
| Ibarra Sections 3.2/3.4 clique-tree updates | `data_.delete_edge` / `data_.insert_edge`, linked through hooks | Section 4 direct-vector replacement now used in A2; correct on tests; two high-degree cliques can still induce linearly many ST operations on **Insert** |
| Ibarra Section 5.1 matrix W | `W_` with O(n) row formulas | Functional tests pass; initial W is an n-by-n matrix |
| Ibarra Section 6 accelerated deletion | `enable_fast_delete_queries` | A2+ CLI smoke and differential checks pass; inherits the A2 blockers |
| Ibarra Section 7 preprocessing | `data_(Graph(n))` and reverse-PEO insertions | Actual cost differs from the A2 table; see Section 14.5 |

### 14.3 BLOCKER B1 — A2 insertion still lacks an O(n) proof after Section 4 optimization

**Source:** Ibarra (2008), Section 3.4 describes inserting a
temporary maximal-clique node `z` and contracting either/both of
the old adjacent cliques when they cease to be maximal.
**Section 4 explicitly permits a simpler equivalent**: directly
enlarge the characteristic vector of a subsumed clique, rather
than create a temporary node and subsequently contract it.

**Source-faithful correction implemented:** On the A2 research
path only, `src/dynamic_ibarra.cpp::insert_edge` now applies
Ibarra Section 4's direct vector replacement. If exactly
one clique is subsumed, it is updated in place; if both
are subsumed, a surviving clique is selected and only
the other clique is contracted. Selection of the
higher-degree surviving clique avoids unnecessary tree
operations; it is an implementation choice within the
paper's permitted arbitrary contraction choice, **not**
an unreferenced substitute dynamic tree.

**Measured improvement:** Earlier A2 code introduced a
temporary node and contracted both sides. On star
graphs, this required 2n-1 ST link/cut calls for one
valid insertion (historical baseline recorded in
`benchmarks/results/a2_st_insert_operation_counts_2026-11-19.csv`).
Under the Section 4 implementation the same input
requires only one cut, for every tested n=12 to 384.

**Residual adversarial obstruction:** On the chordal
double-star tree, with central cliques `{r,u}` and
`{r,v}`, each having many attached maximal cliques,
inserting edge `{u,v}` forms a triangle and removes
both central cliques. A valid clique tree has both
central cliques of high degree. The code must move the
neighbors of the removed clique.

A seeded search over vertex relabelings found this
**concrete, reproducible** configuration:

| n | Degree of each subsumed clique | ST links | ST cuts | ST total |
| ---: | ---: | ---: | ---: | ---: |
| 27 | 13 | 12 | 13 | **25** |
| 51 | 25 | 24 | 25 | **49** |

This shows that even after the published Section 4
simplification, the implementation can make
**Theta(n) dynamic-tree primitive calls** in one
permitted insertion. At n=99 the tested seed's
best configuration had degrees 49 and 2, and only
three ST calls; the examples do not prove that
all clique trees force linear calls.

Each ST primitive has a published **worst-case
O(log n) upper bound** under its invariants
(Sleator–Tarjan 1983, Theorem 8). The current direct
cost accounting is therefore **O(n+r log n)** for
r reassigned edges, up to **O(n log n)** with r=Theta(n),
and **does not establish** Ibarra Section 7's
claimed O(n) worst-case insertion time.

**Critical distinction:** Theta(n) calls times a
worst-case O(log n) bound is an O(n log n)
*upper-bound analysis*, NOT proof that the
runtime is Omega(n log n). A sharper aggregate
proof could conceivably give O(n), but neither
Ibarra Sections 3–5 nor the inspected
Sleator–Tarjan primitive bound supplies that
aggregate contraction theorem. We must not
invent one or mark the target met without a
published or independently validated proof
that preserves the specified source method.

Reproducible source:
`benchmarks/a2_operation_audit.cpp`,
`benchmarks/a2_two_hub_stress.cpp`.
CSV results:
`benchmarks/results/a2_section4_insert_st_calls_2026-11-19.csv`,
`benchmarks/results/a2_two_hub_search_2026-11-19.csv`.
Release workflow:
https://github.com/khemsinghbhati/MTP-3rd-sem/actions/runs/44581193992.

**Acceptance:** **BLOCKED** on the required worst-case
O(n) insertion bound. No amortized splay, HLD, alternate
top tree, or unreferenced batched contraction is used.

### 14.4 FIXED F1 — deterministic Insert-Query adjacency lookup

The research-branch graph representation uses
`std::set<int>` for neighbor membership, rather than
`std::unordered_set<int>`, in
`include/mtp/graph.hpp`. C++ ordered sets are
balanced search trees; `Graph::has_edge` is therefore
**O(log n) worst-case**, as are set insertion/erasure
(excluding the separately counted adjacency-vector
erase work).

A2's `Insert-Query` performs a constant number of
such membership tests and O(log n) indexed
path operations, each O(log n) worst-case if the
published Sleator–Tarjan representation invariants
hold. Thus the graph lookup no longer prevents the
advertised O(log² n) worst-case *algorithmic*
query bound. This does **not** substitute any
different chordal-graph algorithm.

The explicit `tests/test_deterministic_graph_lookup.cpp`
compares updates and lookups against a dense
independent adjacency matrix and verifies edge counts
and neighbor lists. Full A1/A2 regression tests pass
with the replacement in the Release workflow.

**Side effect:** inserting the input's m edges via
the `Graph` container has an O(m log n) worst-case
balanced-set component, rather than an average-case
constant-time hashing component. This is separately
accounted for in initialization; the MCS and
Blair–Peyton algorithms still traverse the
existing adjacency lists in O(n+m).

### 14.5 VERIFIED SOURCE/REPRESENTATION DISCREPANCY — preprocessing

**Input-independent lower-bound witness:** Take an
empty graph on n vertices, so m=0 and k=n
singleton maximal cliques. Our actual representation
allocates n dense characteristic vectors of length n:
**n² byte entries**, already Omega(n²) initialization
and storage. A2 additionally allocates the matrix
`W_` as **(n+2)² integers**, also Omega(n²)
initialization and memory.

The lower bound is structural and does **not**
depend on benchmarking. It conflicts with the
paper's Section 7 **A1 O(m+n)** and
**A2 O(m+n log n)** preprocessing claims when
m=0, if those bounds include instantiating
the dense representations explicitly specified
in Sections 4 and 5.1. Section 5.1 independently
describes matrix initialization using reverse-PEO
incremental insertion in O(mn) time, but the
materialized W matrix by itself takes Omega(n²)
time/memory; hence a safe accounting must
include the additional n² term.

**Our implementation:** Preprocessing performs
Blair–Peyton initial clique-tree construction,
dense characteristic-vector materialization,
dynamic-forest construction, dense matrix W
initialization, then inserts the initial m edges
one by one in published reverse-PEO order.
The conservative derived upper bound for the
whole *research A2 constructor* is
O(n² + mn log n) from the explicitly measured
representations plus up to O(n log n)
ST work per inserted edge. This is an
upper bound, not an exact tight bound; input
`Graph` construction costs and allocator effects
are separate.

**Measured Release experiment, m=0** (the matrix
column includes the two reserved clique slots):

| n | Characteristic entries | W entries | W bytes | Construct ms |
| ---: | ---: | ---: | ---: | ---: |
| 100 | 10,000 | 10,404 | 41,616 | 0.200 |
| 200 | 40,000 | 40,804 | 163,216 | 0.428 |
| 400 | 160,000 | 161,604 | 646,416 | 1.029 |
| 800 | 640,000 | 643,204 | 2,572,816 | 3.182 |
| 1600 | 2,560,000 | 2,566,404 | 10,265,616 | 10.286 |

Timing is observed on a GitHub Actions runner; it
is **not** a worst-case proof. The entry counts,
however, are exact for this code.

Source:
`benchmarks/a2_preprocessing_audit.cpp`;
recorded:
`benchmarks/results/a2_empty_preprocessing_2026-11-19.csv`.
Passing Release run:
https://github.com/khemsinghbhati/MTP-3rd-sem/actions/runs/44581415789.

**Audited disposition:** The discrepancy is
**verified and documented**, not "fixed".
Reconciling the paper's stronger preprocessing
table would require a published representation
with different initialization/storage accounting.
The research policy disallows fabricating one.
For any MTP presentation, quote the paper's
claimed bound separately from this code's
verified complexity; do not silently equate them.

### 14.6 Correctness and safety evidence, and its limits

The code has two separate independent functional checks:

1. ST forest test: randomized sequences of link/cut/evert
   and kth-node/minimum-weight path queries are checked against
   a simple BFS adjacency-list oracle; biased-tree rank and
   global-bias invariants are checked.
2. Ibarra A2 test: insert/delete queries and permitted
   updates are compared against the A1 implementation.
   Updated maximal cliques, clique-tree edge weights,
   spanning-tree edge counts, and running-intersection
   connectivity are reconstructed and checked independently.

Release tests passed, including Section 4 clique reuse, repeated two-hub cycles, and A2/A2+ CLI samples,
in https://github.com/khemsinghbhati/MTP-3rd-sem/actions/runs/44582444381.
A later Release run including the operation-count audit
also passed. Memory-sanitizer runs and Release runs must
continue to be reviewed at the specific commit accepted.

**These tests cannot prove** every globally biased tree
implementation detail or all worst-case bounds. In
particular, the custom translation's recursion,
split/join rank updates, and dynamic path-set maintenance
need explicit published invariant preservation arguments.
The tested small and random cases are evidence, not proof.

### 14.7 Acceptance gates and disposition — November 19, 2026

| Acceptance item | Result | Evidence |
| --- | --- | --- |
| Functionally source-faithful A2/A2+ on tested inputs | **PASS** | Published Section 4 direct clique-vector replacement; independent A1 differential tests including repeated two-high-degree insertion/deletion cycles |
| Deterministic adjacency lookup in Insert-Query | **PASS** | `Graph::has_edge` now `std::set`, O(log n) worst-case |
| Reproducible A2 insertion-operation experiment | **PASS, BUT BOUND BLOCKED** | Star single-cut improvement; two-hub case at n=51 still requires 49 ST calls |
| Explicit initialization/storage accounting | **VERIFIED SOURCE DISCREPANCY** | Even m=0 allocates Omega(n²) characteristic-vector and W entries, whereas Section 7 lists O(n log n) A2 preprocessing |
| Published ST worst-case invariant proof for this translation | **NOT FULLY CERTIFIED** | Functional + rank checks pass; formal worst-case helper/aggregate proof remains |
| Ibarra A2 insertion O(n) worst-case | **NOT VERIFIED** | Source and implementation do not supply a proved aggregate O(n) bound for linearly many ST operations |
| Complete paper-faithful reproduction A2/A2+ | **BLOCKED** | Two missing worst-case proof/representation requirements |

**No substitutes rule:** No HLD alternative, amortized
splay replacement, alternate top tree, or newly invented
batched contraction is accepted as the final A2.

**Merge gate:** The source-fidelity requirement applies
to *both functional output and worst-case complexity*.
Do not merge the A2 research branch into production
`main` until insertion and preprocessing claims
are reconciled by a verified published method.
The A1/A1+/split-graph implementations on `main`
remain intact.

**Overall audit outcome:** The specified implementation,
lookup correction, regression suite and empirical
analysis are recorded, but the **O(n) Insert claim is
still not proved**, and the explicitly prescribed
dense W/characteristic representation has a
demonstrable initialization lower bound contradicting
the paper's A2 preprocessing summary. These are
research acceptance blockers, not merely testing
or compiler errors.

### 14.8 Final empirical A1 vs **actual ST-backed A2** comparison

A Release benchmark on path graphs distinguishes
construction from repeated insertion queries and
compares `DynamicChordalGraph` (Ibarra A1) against
`SleatorTarjanChordalGraphA2` using the actual
published-method ST1983 dynamic forest. The prior
heavy-path experiment remains a separately labeled
research alternative and is **not** used for A2
performance conclusions.

The query uses the two path endpoints (which form
a long chordless cycle if connected) and repeats
the non-mutating Insert-Query 700 times at each size.

| n | A1 construction ms | ST-A2 construction ms | A1 query ms | ST-A2 query ms |
| ---: | ---: | ---: | ---: | ---: |
| 64 | 0.055153 | 3.393333 | 0.000995 | 0.045888 |
| 128 | 0.049923 | 13.087767 | 0.002645 | 0.093860 |
| 256 | 0.096499 | 23.435337 | 0.004042 | 0.083736 |
| 512 | 0.199641 | 61.656551 | 0.007889 | 0.098593 |
| 1024 | 0.475324 | 189.852227 | 0.017170 | 0.122071 |

**Observed:** In this workload the current A2
implementation is **slower** in absolute time
than A1, despite Ibarra's asymptotic comparison
of O(log² n) versus O(n) Insert-Query. This is
plausibly due to a high constant overhead from
the globally biased ST dynamic tree and the
implementation's multiple expose/evert/restore
procedures. We have **not proved** any particular
crossover size, and timings are not worst-case
operation proofs.

Source:
`benchmarks/benchmark_a2_st.cpp`;
CSV:
`benchmarks/results/a1_vs_st_a2_path_2026-11-19.csv`.
The observed data is from the successful Release
workflow:
https://github.com/khemsinghbhati/MTP-3rd-sem/actions/runs/44582444381.

### 14.9 Test evidence and nonmerge decision

After the Section 4 insertion refactor and
deterministic adjacency representation change:

- The dedicated Release workflow passed the ST
  forest oracle; A2/A2+ versus A1 chordal
  differential tests; repeated two-high-degree
  insertion/deletion cycles; A2 and A2+ CLI;
  star/double-star, randomized-relabeled
  two-hub insertion counts; empty graph
  preprocessing footprint; and actual A1-vs-A2
  timing comparison.
- The research branch is **not** merged into
  `main` because its O(n) insertion complexity
  remains unproved and the source/representation
  preprocessing bounds are demonstrably
  inconsistent on m=0. This is a deliberate
  enforcement of the strict published-method
  acceptance policy.
- Results from sanitizer-enabled CI are
  separately tracked and must be recorded
  for the final accepted commit. Passing
  tests does not substitute for a worst-case
  proof of globally biased path-set helpers.


## 15. Published-source O(n) A2 insertion repair — November 20, 2026

### 15.1 References and method status

The earlier O(n log n) direct-composition analysis in Section 14 applies
to the earlier implementation that called the dynamic-tree link/cut
functions independently for every high-degree clique-tree contraction.
The revised code uses the EXACT Sleator–Tarjan globally biased forest,
but for a double-subsumption insertion reconstructs it using a later
published linear-time globally biased tree constructor.

Primary published building blocks:

- Ibarra (2008), Sections 3.4, 4, and 5.1: clique-tree update,
  direct characteristic-vector replacement, and O(n) W row update.
- Sleator and Tarjan (1983), Sections 4–5, Theorem 8: size-based
  heavy-edge partition, the globally biased dynamic tree,
  and O(log n) worst-case forest primitives.
  https://www.cs.cmu.edu/~sleator/papers/dynamic-trees.pdf
- Bent, Sleator and Tarjan (1985), Section 4, Figure 15,
  Theorem 12: O(t) ACTUAL-TIME bottom-up construction of a
  globally biased binary tree on t ordered, weighted items.
  https://www.ime.usp.br/~cris/aulas/14_2_331/projetos/BentSleatorTarjan85.pdf

IMPORTANT SOURCE DISTINCTION: Ibarra does not explicitly describe
rebuilding the entire forest after an insertion. Our method is a
source-supported composition of the original ST83 exact forest with
BST85's published linear construction. It changes the maintenance
schedule in the double-subsumption case, NOT the dynamic-tree
data structure or the chordal graph algorithm. This deviation from
the literal operation schedule must be disclosed in the MTP.

### 15.2 Source-to-code mapping

| Published operation | Implemented location |
| --- | --- |
| BST85 Figure 15 rightmost-leaf ascent, graft, tilt-left and promotions | GloballyBiasedBinaryTree::build_linear |
| O(1) positive integer rank computation on the word RAM | GloballyBiasedBinaryTree::floor_log2 (five fixed shifts) |
| ST83 Section 4 heavy edge: 2 size(child) > size(parent) | SleatorTarjanWorstCaseForest::initialize_linear_forest |
| ST83 node weight: size(v) minus heavy-child size | initialize_linear_forest leaf weights |
| ST83 globally biased solid path representation | paths_.build_linear and path_root_ |
| ST83 dashed path sets ordered by decreasing path size | counting-sort weights [1,N] then sets_.build_linear |
| Ibarra Section 4 subsumed clique-vector replacement | DynamicChordalGraph::insert_edge |
| Source-based O(N) dynamic forest regeneration for two absorbed cliques | SleatorTarjanChordalGraphA2::insert_edge |
| ST83 published O(log n) link/cut/evert/path queries | unchanged SleatorTarjanWorstCaseForest methods |

### 15.3 Proof of worst-case O(n) A2 Insert

Let n be the number of original vertices and N=n+2 the fixed
clique-node capacity. A chordal graph has at most n maximal cliques,
so a valid clique tree has O(N) vertices and O(N) edges.

LEMMA (O(N) exact ST forest construction). The static constructor
build_linear_forest on a forest of N nodes uses O(N) worst-case
word-RAM operations and O(N) auxiliary memory.

PROOF:
1. Adjacency lists, DFS parents, subtree sizes, and the ST83
   size-heavy child of each node are computed with O(N) edge
   and vertex visits. Isolated nodes need no special handling.
2. For each vertex v the external leaf weight is defined as
   size(v)-size(heavy_child(v)) if the latter exists, otherwise
   size(v). All weights are positive integers in [1,N].
   The leaf rank floor(log2 weight) is computed in constant
   time using fixed word-sized shifts, not repeated division.
3. The maximal solid paths partition the N vertices. Every
   vertex is included in exactly one path leaf list. Applying
   BST85 Figure 15 and Theorem 12 to each such list costs
   O(sum of path lengths)=O(N), including a one-time
   aggregate postorder traversal for each resulting tree.
4. Each dashed edge contributes one entry to exactly one
   parent path set. There are at most N-1 such entries.
   A path's weight equals size(tail(path)) and lies in [1,N].
   Bucket counting-sort across all dashed paths is O(N).
   BST85's O(t) constructor applied to each sorted path
   set costs O(sum of path-set sizes)=O(N).
5. The newly constructed globally biased tree has O(N)
   allocated nodes. Destroying the previous representation
   during replacement costs O(N).
Thus the total static construction and replacement cost is
worst-case O(N)=O(n). QED.

CASE 1 (no old clique becomes non-maximal): Ibarra's update uses
O(1) dynamic-tree link/cut operations. With ST83's O(log n)
bound, these add O(log n); all vector and W-row work is O(n).

CASE 2 (exactly one old clique becomes non-maximal): Section 4's
direct characteristic-vector expansion similarly performs only
O(1) ST operations. Remaining clique-tree and W work is O(n).

CASE 3 (both old cliques become non-maximal): The clique-tree
contraction may redirect Theta(n) neighbors. The revised code
temporarily detaches the ST update hooks, performs the correct
O(n) clique-tree adjacency-list transformation, collects its
O(n) active edges, and applies the lemma to reconstruct the
SAME ST83 globally biased forest in O(n). It reattaches the
hooks only after the new representation is complete.
The published Section 5.1 W-row update is O(n).

All three insertion cases, including the Insert-Query precheck
(O(log^2 n), which is O(n)), therefore take O(n) worst-case
in the specified conventional fixed-word RAM model.
The optional Section 6 A2+ index retains its additional
O(m+n) insertion-update cost.

### 15.4 Adversarial operation counts and tests

In the original double-star counterexample, the former code
issued 25 ST primitives at n=27 and 49 primitives at n=51.
The new source-based static-ST reconstruction gives:

| n | Former ST link/cut calls | New ST link/cut calls | Counted linear-build work |
| ---: | ---: | ---: | ---: |
| 27 | 25 | 0 | 358 |
| 51 | 49 | 0 | 692 |
| 99 | 3 | 0 | 1349 |

The measured work is consistent with the published linear
construction, but the proof is the sum-of-path-size argument
above plus BST85 Theorem 12, not a timing extrapolation.

Verification files on research/a2-linear-st-construction:
- tests/test_bst85_linear_builder.cpp: weighted rank,
  neighbor-bias, order, minimum path edge, and work counts.
- tests/test_linear_st_forest.cpp: exact source-based ST
  size partition with BFS path oracle, followed by dynamic
  link/cut/evert operations.
- tests/test_a2_linear_insertion.cpp: repeated chordal
  double-star insert/delete cycles, all source invariants,
  and independent A1 differential query checks.
- tests/test_ibarra_a2.cpp: previous exhaustive and
  randomized chordal correctness tests, also A2+.

LIMITATION REMAINING: The published preprocessing table
in Ibarra Section 7 is inconsistent with materializing
an n-by-n matrix W and n full length-n vectors on m=0.
See Section 14.5. The present repair establishes the
per-insertion worst-case bound; it does NOT claim to
solve that separate source/preprocessing discrepancy.

### Implementation evidence

- **Modular C++ implementation:** `include/mtp/` and `src/`, including
  static clique graph and clique-tree modules.
- **Correctness and regression tests:** `tests/test_algorithms.cpp` and
  `tests/test_clique_tree.cpp`.
- **Reproducible build and CI:** `CMakeLists.txt` and
  `.github/workflows/ci.yml`.

---

## 16. Primary-source reconciliation of preprocessing — November 20, 2026

### 16.1 Scope and exact published claims

This audit distinguishes **an original paper's claimed bound** from
**a proved bound for the actual representation**. It is not an alternative
algorithm implementation. The user's no-substitutes rule is binding.

Ibarra (2008), *Fully Dynamic Algorithms for Chordal Graphs and Split
Graphs*, ACM Transactions on Algorithms 4(4), Article 40:

- **Section 7**, printed page 40:17: A1 preprocessing
  `O(m+n)`, A2 preprocessing `O(m+n log n)`, and A1+/A2+
  preprocessing `O(mn+n^2)`.
- **Section 5.1**, printed page 40:13: for A2 the paper explicitly
  maintains an **n by n matrix** `W(x,y)=|K_x intersect K_y|`.
  It states that computing the initial matrix by comparing every
  pair of maximal cliques costs `O(n^3)`. The offered alternate
  initialization starts from an empty graph and performs all m
  reverse-PEO edge insertions, costing `O(mn)`.
- **Sections 3–4** explicitly represent each maximal clique using
  a length-n characteristic vector, with constant-time membership.
- Ibarra's 2001 PhD thesis, *Dynamic Algorithms for Chordal and
  Interval Graphs*, University of Victoria, Section 2.5.1 (printed
  page 28–29), also explicitly states that computing the dense W
  dominates preprocessing by `O(n^3)`. The earlier author source
  confirms the dense-matrix cost rather than providing a faster
  replacement.

Primary source links:

- Published article DOI: https://doi.org/10.1145/1383369.1383371
- 2001 author thesis: https://dspace.library.uvic.ca/bitstreams/174b0bfe-c154-4896-b114-a88441d79e29/download

### 16.2 Concrete worst-case witness: empty graph

Let `G` be the empty graph with n vertices and m=0. Its n
maximal cliques are singleton sets, so `k=n`. The literal
current C++ representation initializes:

1. n characteristic vectors of length n, i.e. **n squared byte
   entries** in `DynamicChordalGraph::DynamicChordalGraph`.
2. `W_` with `(n+2)^2` integer entries in
   `SleatorTarjanChordalGraphA2::initialize_weights`.

Thus actual initialization has a lower bound of:

```math
T_{A1}(n,0)=\Omega(n^2),\qquad
T_{A2}(n,0)=\Omega(n^2).
```

For this same graph, the Section 7 numbers would be
`O(n)` for A1 and `O(n log n)` for A2. These are incompatible
as n grows. This is an **asymptotic counterexample for a
literal initialized dense representation**, not a timing estimate.

A faster MCS or Blair–Peyton constructor does not remove these
costs: it builds the clique tree itself in `O(m+n)`, not all
the dense characteristic-vector and W entries.

The precise limitation is conditional: this does **not** prove that
no other representation or algorithm could achieve the table's
bounds. It proves the table's bounds are unattainable by initializing
the full, explicitly dense structures currently implemented.

### 16.3 Why the paper's reverse-PEO alternative does not fix it

The reverse-PEO route in Section 5.1 makes every intermediate
graph chordal, but it performs m individual `Insert` operations.
Charging each by the paper's worst-case `O(n)` yields
`O(mn)` construction, not `O(m+n log n)` on general graphs.
The paper itself states `O(mn)` for this alternative.

For the current C++ A2 constructor, the actual work includes
the dense initial representations, dynamic-tree initialization,
then m published reverse-PEO updates. The previous Section 14.5
conservative `O(n^2 + mn log n)` upper-bound accounting remains
valid as an implementation-oriented upper bound, but is not
claimed tight. There is **no** verified `O(m+n log n)`
end-to-end constructor in the repository.

### 16.4 Later publication search and source-fidelity gate

A relevant later source, Banerjee, Raman and Satti,
*Maintaining Chordal Graphs Dynamically: Improved Upper and Lower
Bounds*, CSR 2018, DOI:
https://doi.org/10.1007/978-3-319-90530-3_4,
provides different chordal-maintenance results focused on
decremental edge updates. Its published talk explicitly labels
the results as decremental. This does not give a drop-in
initialization of Ibarra's exact fully dynamic Section 5.1 W
structure with the four A2 operation bounds. Other papers
with faster individual chordal operations must likewise not
be represented as having repaired the specific W initialization.

A distinct *published data-structure building block* was also checked:
Hagerup and Kammer, *On-the-Fly Array Initialization in Less Space*,
ISAAC 2017, DOI:
https://doi.org/10.4230/LIPIcs.ISAAC.2017.44.
It gives constant-time initialization and access to an array
in a word-RAM model using uninitialized memory. Although it could
address the **zero-filling** cost of an already allocated
characteristic array or W, it does **not** calculate the
nonzero intersection values of W, does **not** remove the
quadratic array's storage reservation, and provides no
published theorem establishing Ibarra's A2 end-to-end
O(m+n log n) preprocessing with unchanged operations.
Consequently, using it alone would not pass this acceptance gate.

**Acceptance gate (NOT PASSED):** to replace current preprocessing,
a *published, checked* method must establish all of:

1. Construct the intended maintained structure on an **arbitrary
   chordal input graph** in worst-case `O(m+n log n)` (and A1
   in `O(m+n)` where claimed), accounting for graph loading
   and every allocated/initialized member.
2. Preserve Ibarra A2's **worst-case `O(1)` weight lookup** used in
   the `O(log^2 n)` insertion query and **`O(n)` weight-update**
   work (and its remaining published operation bounds).
3. Preserve constant-time clique-membership tests used inside
   the original query bound, plus the actual Sleator–Tarjan
   worst-case dynamic forest.
4. State the precise source, theorem and computational model,
   map it to production code, test it independently, and audit
   memory plus total preprocessing work across sparse and
   dense chordal inputs.

No verified source satisfying these conditions was identified
in this audit. Consequently **no substitute representation has
been merged**, and the Section 7 preprocessing-table bound
is **not certified**. Claiming it after simply removing the W
matrix, caching values lazily, or deferring initialization would
silently change the `O(1)` lookup obligation and would not
meet the stated method requirement.

### 16.5 Auditable conclusion

- **A1 clique-tree preprocessing only:** Blair–Peyton
  `O(m+n)` as previously tested.
- **Complete A1 dynamic object:** the literal k length-n
  characteristic vectors cost `\Theta(nk)` to initialize.
- **Complete A2 dynamic object:** the literal W matrix
  incurs at least `\Omega(n^2)` initialization even on m=0,
  with additional reverse-PEO inserts.
- **A2 insertion operations:** Section 15's separate
  source-backed `O(n)` repair remains in place; this source
  discrepancy concerns **initialization**, not that repair.
- **Publication fidelity:** without a published replacement
  meeting all above requirements, the correct outcome is
  a documented source-vs-representation conflict, not an
  unverified complexity claim.

---

## 17. Four user-supplied PDFs: theorem-level compatibility audit — November 20, 2026

This review checks whether any of the four supplied research documents
gives an independently published way to meet **Ibarra A1 preprocessing
O(n+m)** or **Ibarra A2 preprocessing O(m+n log n)** on arbitrary
chordal graphs, preserving the A2 **O(1) clique-pair intersection
lookup**, **O(log^2 n) Insert-Query**, and the full update bounds.
The documents were read in their relevant algorithm sections,
not classified by title alone. Here `k` is the *maximum clique
size*, not the number of maximal cliques.

### 17.1 Niranka Banerjee (2021), PhD thesis, Chapter 6

*Dynamizing Graph Classes and Output Sensitive Fault Tolerant
Subgraph Problems*, Homi Bhabha National Institute, Chennai.
This thesis incorporates Banerjee, Raman and Satti (CSR 2018),
*Maintaining Chordal Graphs Dynamically: Improved Upper and
Lower Bounds*, DOI
https://doi.org/10.1007/978-3-319-90530-3_4.

Relevant exact results (uploaded PDF physical page / thesis page):

- **Theorem 6.1.3** (143 / 117):
  `O(n k^2)` preprocessing; `O(1)` deletion feasibility query,
  `O(n+k^2)` deletion update. The structure keeps
  edge-to-clique incidence counts and two-way pointers.
  **Its updates are deletion-only**; it does not give an
  A2 insertion-query or its `W(x,y)` lookup.
- **Theorem 6.1.4** (147 / 121):
  `O(n^2/t + k^2)` amortized cost for a sequence of
  `t` deletions. This is neither the desired
  worst-case fully dynamic bound nor an A2 preprocessing fix.
- **Theorem 6.2.2** (150 / 124):
  from an *already given adjacency list and adjacency matrix*,
  construct a PEO in `O(n+m)`; test deletion in
  `O(min(deg(u),deg(v)))`, update in
  `O(deg(u)+deg(v))`. Its claim is conditional
  on the supplied matrix; creating a dense n by n matrix
  from a sparse edge list is itself Omega(n^2) work.
- **Lemma 6.2.4 and Theorem 6.2.5** (154–156 / 128–130):
  augment each clique-tree edge between bags A,B with
  `A\\B` and `B\\A` in `O(nk log k)` preprocessing;
  maintain them through permitted edge insertions/deletions
  and obtain a PEO in `O(n)` per successful update.
  **Useful published extension** if the project adds
  maintained PEO output, but its preprocessing is not
  `O(m+n log n)` uniformly and its separator differences
  are only for *adjacent* clique nodes, not all pairs.

**Source-method conclusion:** The neighboring-clique
intersection data can be initialized alongside Blair–Peyton,
but Ibarra A2 calls `W(x,y)` for **arbitrary closest clique
nodes** in an Insert-Query; storing only adjacent edge
separators is not an O(1) replacement.

**Concrete insufficiency of neighboring edge *weights*:**
consider a three-clique path. In example A its cliques are
`{a,b,x}`, `{a,b,c}`, `{a,b,y}`, and in example B they
are `{a,b,x}`, `{a,b,c}`, `{b,c,y}`. Both have
intersection weights 2 and 2 on the two clique-tree
edges, but the two endpoint cliques have intersection
size 2 in A and size 1 in B. All listed cliques are
maximal and satisfy the connected-subtree property.
Thus the adjacent separator **cardinalities** do not
determine a general `W(x,y)`; using full adjacent set
differences to reconstruct it may require path work,
for which Banerjee gives no worst-case O(1) A2 query
theorem. Banerjee provides no
published theorem transforming that restricted representation
into A2's required general `W` interface and bounds.
No A1/A2 preprocessing fix is established by this thesis.

### 17.2 Robert Scheffler (2023), doctoral dissertation

*Ready to Order? On Vertex and Edge Orderings of Graphs*,
Brandenburg University of Technology Cottbus–Senftenberg,
DOI https://doi.org/10.26127/BTUOpen-6301.

- **Lemma 3.3.12** (PDF pages 161–162, printed 145–146):
  with n objects already sorted by integer keys in [1,k],
  a block/array structure can be built in `O(n)`
  and then supports **increment/decrement of one key
  by one** and deletion of the minimum/maximum-key
  object in `O(1)). It notes a similar structure
  used by Ibarra. This is a sortable degree/weight
  **bucket-list primitive**, NOT a constant-time lookup
  of `|K_x intersect K_y|` over arbitrary clique pairs.
- **Theorem 3.3.13** (PDF pages 163–166):
  `O(n+m)` degree-minimal weighted-edge elimination
  orderings under the paper's specified setup;
  not a dynamic chordal clique-intersection representation.
- **Chapter 4**, especially Theorem 4.4.7 (PDF
  page 187): fully dynamic recognition and compact
  certificates for **chain graphs**, with `O(n+m)`
  initialization and `O(1)` single-edge updates;
  the same chapter develops **threshold graphs**.
  These are different graph classes, not fully
  dynamic chordal A2.

**Conclusion:** A credible reference for an ordered-bucket
auxiliary structure, useful only if a matching subroutine
is identified. It is *not* a source-backed general W
replacement; none is identified.

### 17.3 Desuo Neto et al. (2026), Entropy 28(3), 293

*Learning Continuous Decomposable Models Using Mutual
Information and Statistical Copulas*, DOI
https://doi.org/10.3390/e28030293.

- **Theorem 1, Eq. (6)** (PDF page 9): represents
  mutual information using clique and separator
  copula-entropy contributions. This is a statistical
  scoring/factorization identity, not the number of
  vertices shared by two maximal cliques.
- **Section 3.4** (PDF pages 16–17):
  uses Ibarra-style chordality-preserving insert/delete
  criteria on a junction tree. The implementation
  describes **splay-tree** link/cut and path queries in
  **amortized** `O(log d)`, with candidate-edge
  enumeration in amortized `O(d^2 log d)`.
  This *changes* Ibarra A2's required worst-case
  ST83 mechanics and does not give an initial `W`
  construction with the target complexity.
- The symbol `W` used for an experimental adjacency
  matrix in this statistical article is **not**
  Ibarra's clique-pair intersection matrix.

**Conclusion:** Confirms the practical relevance of
dynamic chordality-preserving junction-tree moves,
but supplies neither the demanded preprocessing
algorithm nor a compatible worst-case representation.

### 17.4 Louis Ibarra (2001), doctoral dissertation

*Dynamic Algorithms for Chordal and Interval Graphs*,
University of Victoria, Section 2.4 and 2.5.1
(uploaded PDF pages 36–39, printed 26–29).
Author's archival copy:
https://dspace.library.uvic.ca/bitstreams/174b0bfe-c154-4896-b114-a88441d79e29/download.

- **Section 2.4** (PDF page 36) explicitly says the
  first implementation uses an **n-bit characteristic
  vector for each maximal clique**, and labels clique
  tree preprocessing `O(m+n)`. That statement
  does not separately analyze filling all k vectors.
- **Section 2.5.1** (PDF page 38, printed 28)
  explicitly states that the second implementation
  keeps an **n by n matrix W** for
  `W(x,y)=|K_x intersect K_y|`, giving `O(1)`
  query access, and that preprocessing is
  **dominated by O(n^3) construction of W**.
  This passage was inspected visually to check the
  superscript (the scanned text OCR is noisy).
- Subsequent paragraphs derive `O(n)`
  maintenance of the new/modified W row under
  accepted clique-tree updates.

**Conclusion:** The original author himself
acknowledged the cubic direct-W construction in
2001. The 2008 article Section 5.1 describes
`O(n^3)` direct or `O(mn)` reverse-PEO construction,
whereas Section 7 lists `O(m+n log n)` for A2.
The uploaded thesis **corroborates**, rather than
repairs, this inconsistency.

### 17.5 Overall compatibility / acceptance decision

| Uploaded source | Applicable published result | Meets A2 preprocessing? | Meets A2 exact query and update method? |
| --- | --- | --- | --- |
| Banerjee 2021 | Adjacent separator differences; PEO maintenance; deletion-only chordality methods | **No**: O(nk log k) augment or O(nk^2) separate index | **No** O(1) arbitrary-pair W supplied |
| Scheffler 2023 | O(n) bucket construction, O(1) unit-key changes | **No** | **No**, different maintained object |
| Desuo Neto et al. 2026 | Local copula scores with junction trees | **No** | **No**, amortized splay implementation |
| Ibarra 2001 | Exact original dynamic clique-tree approach; dense W | **No**: explicitly O(n^3) W construction | **Yes** original operation framework, but no desired preprocessing |

**Final audit status:** None of these four documents
furnishes the missing published `O(m+n log n)`
general-input A2 initializer with `O(1)`
general clique-pair intersection lookup and all
worst-case dynamic operation guarantees. It would
violate the user's source-fidelity rule to implement
one of these methods and advertise that stronger
preprocessing result. Existing production code
remains untouched.

**Additional accounting caveat:** The current input
`Graph` uses `std::set` adjacency membership,
which has deterministic `O(log n)` per-edge
insertion. On a raw m-edge input list, this can
add `O(m log n)` to preprocessing separately
from the characteristic-vector and W concerns.
Any claimed end-to-end A1 `O(m+n)` repair must
also reconcile that graph-input model (for example
by stating an already-built adjacency-list input
assumption, only if that assumption is explicitly
adopted and proved).

**Next source-specific proof obligation:** Find a
published data structure that supplies general
`|K_x intersect K_y|` **in worst-case O(1)**
after initial preprocessing `O(m+n log n)`
on arbitrary chordal graphs, and allows Ibarra's
`O(n)` row updates without secretly materializing
quadratic initialization or paying per-query
intersection scans. No such theorem appears in
the four reviewed files.

---

## 18. Independent audit of uploaded Claude report — November 21, 2026

**Status:** independently checked research argument, NOT an original published
chordal-graph theorem and NOT a certified resolution of Ibarra Section 7.
This section incorporates the user-supplied 30-page report
*Preprocessing Complexity of Ibarra's Fully Dynamic Chordal Graph
Algorithms*, its Python verification scripts and R1 patch.
It is separate from published sources in Sections 15–17.

### 18.1 Boolean matrix multiplication (BMM) reduction: conditional barrier

Given Boolean u by u matrices A,B, form a chordal graph as follows.

1. The central clique U contains sets E={e_1,...,e_u},
   D={d_1,...,d_u}, F={f_1,...,f_u}, hence |U|=3u.
2. For each row i, attach a simplicial leaf p_i adjacent to
   R_i={e_k:A_ik=1} together with the first
   u-|{k:A_ik=1}| vertices of D. Hence |R_i|=u.
3. For each column j, attach a simplicial leaf q_j
   adjacent to Q_j={e_k:B_kj=0} union D together with
   the first u-|{k:B_kj=0}| vertices of F.
   Hence |Q_j|=2u.
4. There are 5u vertices and Theta(u^2) edges, constructed
   in O(u^2) time. Eliminating all p_i,q_j and then
   the central clique is a PEO, so the graph is chordal.

An insertion (p_i,q_j) preserves chordality iff R_i is
a subset of Q_j:

- If inclusion holds, the new neighborhood
  R_i union {q_j} is a clique; p_i is still simplicial,
  and removing it leaves a chordal graph.
- If inclusion fails, choose r in R_i\\Q_j and
  s in Q_j\\R_i (possible because |Q_j|>|R_i|).
  Then p_i-r-s-q_j-p_i is an induced chordless 4-cycle.

By construction, R_i is contained in Q_j iff no
k satisfies A_ik=B_kj=1. Thus Insert-Query(p_i,q_j)
returns YES iff (Boolean AB)_ij=0.

For any hypothetical preprocess P(n,m) and
Insert-Query Q(n), this yields:

```math
T_{BMM}(u)=O(u^2+P(5u,\\Theta(u^2))+u^2 Q(5u)).
```

In particular, the A2 Section 7 preprocessing
O(m+n log n) and Section 5 worst-case Insert-Query
O(log^2 n) would together imply O(u^2 log^2 u)
Boolean matrix multiplication. This is a **valid
conditional reduction**, not an unconditional lower
bound: a near-quadratic BMM algorithm is not known
to be impossible. The reduction and its test code
are independent audit work, not a theorem sourced
from Ibarra or another published paper. It must
not be attributed to those authors.

A sparse-graph corollary can be obtained by adding
u^2 isolated vertices, producing n'=Theta(u^2),
m=Theta(u^2)=Theta(n'). The padded instance no longer
has 5u vertices, but leads to the same u^2-polylog(u)
conclusion if the hypothetical guarantees are
uniform for arbitrary chordal graphs.

### 18.2 Independently executed Python checks

From the user-supplied `verify(2).py`,
`w_updates(3).py`, without modifying their logical
tests, Python/networkx 3.6.1 passed:

- 406 chordal-graph cases, 4,054 insertion queries,
  150 randomized BMM reductions (1,768 queries).
- Ibarra W-insert formulas on 1,171 insertion cases
  and deletion formulas on 5,895 cases.
- Additionally, an independently executed exhaustive
  u=2 BMM check examined all 256 ordered pairs of
  Boolean 2 by 2 matrices and 1,024 insertion queries.

These are evidence of functional correctness of
the tested reduction/formulas, **not** worst-case
proofs or independent C++ validation.

### 18.3 A1 repair status and source standards

The uploaded report proposes replacing each length-n
clique characteristic vector with:

- a member list for each maximal clique;
- inverse per-vertex lists of containing clique IDs;
- O(n) scratch mark arrays.

The incidence bound `sum_K |K| <= m+n`
follows from distinct earliest vertices in a PEO.
Therefore the proposed **data representation** has
linear initial size. A1 operations can use batched
marking and O(n)-time scans. However no production
implementation or independent invariant audit for
the A1 conversion exists yet. The proposed repair is
**theoretical**, not marked code-complete. Its relation
to standard published clique-incidence and sparse-set
techniques must be documented before final acceptance.

The existing input Graph is still built through
per-edge std::set insertions, so O(m+n) end-to-end
input construction must be accounted for separately
or replaced by a verified sorted-range constructor.

### 18.4 R1 A2 research constructor (strictly a corrected bound)

A separate draft PR #3 proposes an isolated
replacement of the **initial A2 constructor**:

https://github.com/khemsinghbhati/MTP-3rd-sem/pull/3

It uses Blair–Peyton's initial clique tree and
the elementary identity

```math
W(x,y)=\\sum_{v\\in V}[v\\in K_x][v\\in K_y].
```

For each vertex v let T_v be the list of cliques
containing it; increment W[x][y] for every ordered
pair (x,y) in T_v by one. This computes the same
Ibarra Section 5.1 matrix, with initialization
`O(n^2 + sum_v |T_v|^2 + n+m)` (excluding
input edge ingestion). The ST forest is built
with the already implemented ST83/BST85 linear
constructor. **The original O(m+n log n)
preprocessing claim is NOT achieved.**

The alternative R2 drops W, computes the desired
intersection on demand, and retains fast
preprocessing at the cost of worst-case O(n)
Insert-Query; it therefore cannot be called
published A2 with unchanged operation bounds.

### 18.5 Pending proof and CI gates

1. **Initial C++ CI gate PASSED** for R1 branch
   commit 5eeef2bad800c5022669aa8f77f5f5d4e6e89604:
   two GitHub Actions C++ verification runs passed
   (runs 44806001144 and 44806107732).
   This verifies existing repository regression tests,
   not a final independent complexity certification.
   Continue dedicated adversarial initialization
   and proof-level auditing before merge.
2. Implement and prove A1 incidence maintenance
   under repeated Delete/Insert, including node
   recycling and optional fast delete index.
3. Keep the independently derived BMM reduction
   explicitly labeled **unpublished and conditional**;
   verify novelty/precedence before making a
   research novelty claim.
4. Do NOT merge a claimed A2 O(m+n log n)
   constructor or call R2 source-faithful without
   a published, full complexity proof.

---

*End of current report snapshot. Future work should be appended to the
relevant sections of this same report, with explicit dates and evidence.*