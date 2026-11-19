# M.Tech Thesis Project — End-Semester Technical Report

**Project:** Chordal Graphs: Foundations, Maximal Cliques, Clique Trees, and Dynamic Algorithms  
**Student:** Khem Singh  
**Program:** M.Tech, Computer Science and Engineering, IIT Bhubaneswar  
**Report updated:** November 19, 2026  
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

### 13.5 Outstanding required variants — not misrepresented as complete

| Version in Ibarra Section 7 | Current status | Published requirement |
| --- | --- | --- |
| A1: first chordal dynamic implementation | Implemented, update tests passing | O(n) for each of four operations |
| A2: second chordal implementation | NOT IMPLEMENTED | O(log² n) insertion query; O(n log n) deletion; O(n) insertion |
| A1+: modified-Yellin accelerated deletion query | Implemented and tested | O(sqrt(m)) query; O(m+n) updates |
| A2+: both optimizations | NOT IMPLEMENTED (A1+ index available) | Both requirements above |
| Section 8 split-graph degree sequence | Implemented, random tests passing | O(1) query and valid edge update |

A2 requires the **worst-case logarithmic dynamic tree** from
Sleator and Tarjan (1983), plus Ibarra's maintained intersection
matrix `W` and vertex-to-clique pointers, all subject to
Sections 5–5.1. A splay-only link-cut tree has **amortized**, not
worst-case, bounds and must not be substituted silently.
A1+/A2+ require the exact member/overflow/common-count structure and
maintenance invariants in Ibarra Sections 6.2–6.3, including the
`O(m)` update bound.

**Acceptance condition:** Finish each optimized implementation, add
independent update-sequence tests, prove the source-to-code mapping and
its time bounds, and measure empirical scaling before marking it
complete. Do not claim complete paper reproduction until then.

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

### 13.7 Remaining implementation dependency: worst-case dynamic trees

**Ibarra (2008), Section 5** requires the operations of a
Sleator–Tarjan dynamic tree: reroot, path minimum, locate the
ith node on a path, link, and cut, with worst-case logarithmic
bounds. The relevant published data-structure reference is:

Sleator, D. D. and Tarjan, R. E. (1983), *A Data Structure for
Dynamic Trees*, Journal of Computer and System Sciences 26(3),
362–391, DOI 10.1016/0022-0000(83)90006-5,
**Section 5 and Theorem 8**.

Crucially, that publication distinguishes an amortized implementation
from a worst-case implementation. The latter uses partitioning by
size and globally biased binary trees. Substituting an ordinary
splay-based link-cut tree does **not** establish Ibarra's
worst-case operation bounds.

Ibarra also specifies in **Section 5.1** an n-by-n
clique-intersection-weight matrix, with O(n) updates after
permitted chordal edge insertions or deletions, and a
vertex-to-clique-node pointer for each original graph vertex.

**Not yet implemented and not claimed:** a worst-case logarithmic
dynamic-tree structure, its integration with the exact Ibarra
Section 5 operations, and its linked O(n)-update weight matrix.
Therefore the A2 and A2+ variants and all-variant
comparative benchmarks remain incomplete.

### Implementation evidence

- **Modular C++ implementation:** `include/mtp/` and `src/`, including
  static clique graph and clique-tree modules.
- **Correctness and regression tests:** `tests/test_algorithms.cpp` and
  `tests/test_clique_tree.cpp`.
- **Reproducible build and CI:** `CMakeLists.txt` and
  `.github/workflows/ci.yml`.

---

*End of current report snapshot. Future work should be appended to the
relevant sections of this same report, with explicit dates and evidence.*