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
> dynamic algorithm. The supplied Ibarra (2008) paper now serves as the
> primary source for the clique graph, clique tree, and planned update rules.

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
The code in this repository currently builds a **static** clique tree and
does not claim any of these dynamic bounds.

### 1.1 Current implementation baseline

The currently implemented foundation comprises:

1. Graph-theoretic preliminaries, including chordal graphs and PEOs.
2. Linear-time Maximum Cardinality Search (MCS) and its correctness argument.
3. Computing maximal cliques from a PEO and reasoning about subsumption.
4. Constructing a clique tree directly from the PEO in worst-case
   $O(n+m)$ time, without enumerating the weighted clique graph.
5. Keeping an explicit clique graph and maximum-spanning-tree method
   **only as a test baseline**, not as the production algorithm.

**Remaining work:** chordality-preserving edge queries, dynamic updates,
and performance experiments.

### 1.2 Project boundaries at this snapshot

- **Implemented:** simple undirected graph API; MCS; deterministic
  $O(n+m)$ PEO verification; maximal-clique extraction; direct
  $O(n+m)$ static clique-tree construction; CLI; automated tests.
  The explicit clique graph remains as a reference test implementation.
- **Not yet implemented:** insertion/deletion queries, incremental
  clique-tree updates, and running-time experiments for dynamic operations.
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
| Direct clique tree | `include/mtp/clique_tree_linear.hpp` | `src/clique_tree_linear.cpp` | $O(n+m)$ clique-tree preprocessing without full $W_G$ |
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
[Ibarra Section 3 edge queries] ------- PENDING
                 |
                 v
[Ibarra Section 3 edge updates] ------- PENDING
                 |
                 v
[Section 4 O(n) implementation] ------- PENDING
                 |
                 v
[Sections 5-6 optimizations] ---------- PENDING

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

### 9.2.3 Production: direct clique tree in worst-case linear time

**Paper alignment.** Ibarra (2008), Section 2 (paper pp. 40:5–40:6)
cites $O(n+m)$ algorithms for initial maximal-clique and clique-tree
computation. Section 4 (p. 40:11) uses this preprocessing.
The paper does *not* give the full preprocessing pseudocode: the
elimination-forest contraction below is our independently implemented
linear-time realization of that required output and asymptotic bound,
not a verbatim extraction from Ibarra.

**Implementation:** `include/mtp/clique_tree_linear.hpp` and
`src/clique_tree_linear.cpp`. The CLI invokes this constructor
directly and never constructs the full $W_G$.

The **elimination forest** is built from a valid PEO by connecting
each vertex $u$ to its earliest later neighbor $p(u)$, if any. Define
candidate cliques $C_u$ as in Section 5. The code performs:

1. Compute positions, parents, and later-neighbor counts by scanning
   adjacency lists.
2. For each child $u$ of $p=p(u)$, if the parent-subsumption cardinality
   condition holds, record $u$ as one witness that $C_p$ is non-maximal.
3. Contract exactly one witnessed edge for each non-maximal candidate.
   The contraction edges form a subforest, so linear-time DFS assigns
   every original candidate vertex to a unique remaining maximal clique.
4. For each *uncontracted* elimination-forest edge from $u$ to $p(u)$,
   connect the corresponding maximal-clique nodes. The separator weight
   equals the number of later neighbors of $u$.
5. For disconnected input graphs, link the component clique trees
   with zero-weight dummy edges, as in the paper.

**Why this works.** For each original vertex $v$, the candidate nodes
containing $v$ form a connected subtree of the elimination forest:
if $v$ is a later neighbor of $u$, then $v$ is also contained in the
candidate of each vertex along the parent chain from $u$ to $v$.
The contraction of an edge for which one candidate is contained in
the other preserves this property.

Every maximal clique is a candidate (Section 5.1). To see why the
witness rule catches *every* non-maximal candidate $C_p$, take a
maximal clique $C_w$ strictly containing $C_p$. Since $C_p$ already
contains every later neighbor of $p$, the representative $w$ must
occur earlier than $p$. The parent path from $w$ to $p$ consists of
candidates containing every vertex of $C_p$. In particular, if $u$
is the child of $p$ on this path, $C_p$ is a strict subset of $C_u$,
which forces the cardinality witness condition. Thus contraction
removes exactly the non-maximal candidate nodes.

Before contraction, the intersection of candidates at a parent edge
$(u,p(u))$ is exactly the set of later neighbors of $u$. Because the
running-intersection property is preserved and each contracted
component's representative contains its removed candidates,
the intersection size across a surviving edge is unchanged. This
gives the correct tree-edge weight without a clique-pair scan.

All adjacency scans, candidate-member copies, witness checks,
contraction edges, DFS steps, and emitted tree edges have total
size $O(n+m)$. Hence, given an adjacency-list representation of
the original chordal graph:

```math
T_{\mathrm{preprocess}}(n,m)=O(n+m)
```

The bound is **worst case** for the construction and PEO verification
code. Building the in-memory graph from externally supplied text is
a separate input-handling step.

**Evidence:** `tests/test_linear_clique_tree.cpp` verifies the
maximal-clique set, correct edge weights, tree connectivity,
running-intersection property, and maximum total intersection weight
against an independent pairwise Kruskal oracle. It checks all
**19,048 chordal graphs on 1–6 vertices**, **800 seeded randomized**
chordal graphs, and larger star and complete graphs. The full pairwise
algorithm is never used inside the constructor.

**Scope:** This fixes the *static preprocessing* deviation. It does
not implement Ibarra's four dynamic operations, his characteristic
vectors, dynamic trees, or the faster deletion-query structure.

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

**Current method audit — not complete:** Our existing
`build_clique_tree_linear()` constructs an elimination forest and
contracts selected parent relationships. The implementation has been
validated computationally, and its own operations have an
O(n+m) analysis. However, its **exact contraction algorithm has not yet
been matched step by step to the pseudocode of Blair–Peyton Figure 4.2
or Habib et al. Algorithm 4**. It must therefore remain a **provisional
research implementation**, not a source-approved reproduction.

**Preferred correction:** Replace or formally map that implementation
to Blair–Peyton's explicitly published expanded-MCS construction, given
that our production graph ordering already uses MCS. Preserve existing
tests as regression checks, not substitutes for algorithm provenance.
Do not claim that merging the two papers' techniques constitutes a
published algorithm unless a publication expressly supports that
combination.

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
| Map direct clique-tree constructor to published algorithm | Blair–Peyton Fig. 4.2 / Habib et al. Algorithm 4 | OPEN — REQUIRED |
| Validate paper's stated complexity bounds from primary text | Section 1.3 checked; dynamic implementations remain pending | PARTIAL |
| Implement dynamic insert/delete queries | Central thesis target | OPEN |
| Implement incremental updates and compare against rebuilding | Central thesis target | OPEN |
| Benchmark runtime/scaling once dynamics are implemented | Experimental evaluation | OPEN |

## 11. Development log

| Date | Milestone | Evidence |
| --- | --- | --- |
| 2026-11-18 | Refactored code into separate C++17 graph, MCS, and clique modules | `src/`, `include/` |
| 2026-11-18 | Added PEO verification, CLI input handling, tests, CMake, CI configuration | Source and test files |
| 2026-11-18 | Exhaustive verification for $n\le6$ and 600 random chordal cases | `tests/test_algorithms.cpp` |
| 2026-11-18 | Added weighted clique graph and static clique tree | `src/clique_graph.cpp`, `src/clique_tree.cpp` |
| 2026-11-18 | Verified 19,048 chordal cases, brute-force maximum tree checks, and 600 random cases | `tests/test_clique_tree.cpp` |
| 2026-11-18 | Replaced production clique-graph enumeration with direct linear clique-tree construction | `src/clique_tree_linear.cpp`, `src/main.cpp` |
| 2026-11-18 | Replaced expected-time hash PEO validation with worst-case linear grouped marking | `src/mcs.cpp` |
| 2026-11-18 | Added 19,048 exhaustive, 800 randomized, and large-graph tests of direct clique tree | `tests/test_linear_clique_tree.cpp` |
| Pending | Dynamic queries and incremental updates | Not claimed as completed |

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