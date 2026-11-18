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

Let the input be a simple, undirected graph:

$$
G=(V,E), \qquad n=|V|, \qquad m=|E|.
$$

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

$$
\forall\,x,y\in K,\quad x\ne y \implies \{x,y\}\in E.
$$

A **maximal clique** cannot be extended to a strictly larger clique:

$$
\nexists\,K' \text{ that is a clique and satisfies } K\subsetneq K'.
$$

A **maximum clique** is a clique with the largest cardinality among all
cliques in the graph. Being *maximal* does not necessarily mean being *maximum*.

### 2.3 Perfect elimination ordering (PEO)

Let a vertex ordering be

$$
\alpha=(v_0,v_1,\ldots,v_{n-1}),
$$

where $\operatorname{pos}(v)$ is the index of vertex $v$ in the ordering.
The **later neighbors** of a vertex $u$ are

$$
N^+(u)=
\left\{v\in N(u)\ \middle|\ 
\operatorname{pos}(v)>\operatorname{pos}(u)\right\}.
$$

An ordering is a **perfect elimination ordering** when *every* later-neighbor
set is a clique:

$$
\boxed{\forall\,u\in V,\qquad N^+(u)\text{ is a clique}.}
$$

**Foundational theorem:**

$$
\boxed{G\text{ is chordal}\iff G\text{ has a PEO}.}
$$

### 2.4 Why a PEO is useful

For every vertex $u$, form a **candidate clique**

$$
\boxed{C_u=\{u\}\cup N^+(u).}
$$

The PEO ensures that all vertices in $N^+(u)$ are pairwise adjacent;
by definition, each is also adjacent to $u$. Therefore $C_u$ is a clique.

**Core connection:** A PEO lets us construct candidate cliques
and then eliminate those that are contained in larger candidates.

## 3. Maximum Cardinality Search (MCS)

### 3.1 Algorithmic idea

For every unnumbered vertex $v$, Maximum Cardinality Search maintains
its **weight**: the number of neighbors already numbered by the algorithm.

$$
w(v)=\left|\{u\in N(v):u\text{ is already numbered}\}\right|.
$$

Initially, $w(v)=0$ for all vertices. MCS builds the ordering **from right
to left**:

1. Select an unnumbered vertex having the largest current weight.
2. Put it in the next free position, starting at $n-1$ and moving to $0$.
3. Mark it as numbered.
4. For each unnumbered neighbor $u$ of that vertex, update

   $$
   \boxed{w(u)\leftarrow w(u)+1.}
   $$

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

$$
\boxed{
G\text{ is chordal}\implies
\alpha=\operatorname{MCS}(G)\text{ is a PEO}.
}
$$

A contradiction-proof overview, attributed to Vandenberghe and Andersen,
proceeds as follows:

1. Assume the produced ordering is not a PEO. Consider a chordless path
   $P=(v_0,v_1,\ldots,v_k)$, $k\ge 2$, such that its interior vertices
   appear earlier in the ordering than its endpoints.
2. Choose such a path to maximize the smaller endpoint position, and
   label the endpoints such that

   $$
   \operatorname{pos}(v_0)>
   \operatorname{pos}(v_k)>
   \operatorname{pos}(v_1).
   $$

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

$$
\boxed{T_{\mathrm{MCS}}(n,m)=O(n+m).}
$$

Space, including the stored graph, is $O(n+m)$.

## 4. Verifying a perfect elimination ordering

The modular implementation verifies the PEO condition before any
maximal-clique extraction.

For a vertex $u$ with nonempty $N^+(u)$, let $p(u)$ be its
**earliest later neighbor**, meaning the vertex with smallest later
position:

$$
p(u)=
\underset{v\in N^+(u)}{\operatorname{arg\,min}}
\ \operatorname{pos}(v).
$$

For every other later neighbor $v$, the verifier requires an edge from
$p(u)$ to $v$:

$$
\boxed{
\forall\,u\in V,\quad
\forall\,v\in N^+(u)\setminus\{p(u)\},
\qquad \{p(u),v\}\in E.
}
$$

When $N^+(u)=\varnothing$, the condition is vacuous. Testing it for all
vertices is sufficient for the PEO property (by induction on the ordering).

The graph uses hash-indexed adjacency membership, with
**$O(1)$ expected time per membership query**. This gives

$$
\boxed{T_{\mathrm{PEO\ verification}}(n,m)=O(n+m)
\quad\text{(expected)}.}
$$

Additional position-array space is $O(n)$; the graph's adjacency index
is already stored in the graph representation.

Invalid-length orderings, repeated vertices, and orderings that fail the
PEO condition are rejected. If the MCS-generated ordering fails, the
CLI reports a non-chordal input and exits with status 2.

**Complexity note:** The hash-based $O(1)$ membership cost is expected,
not a strict adversarial worst-case bound. MCS itself has deterministic
$O(n+m)$ runtime.

## 5. Maximal clique extraction

### 5.1 Candidate cliques

Let $\alpha$ be a PEO and use the notation

$$
\operatorname{Madj}(u)=N^+(u).
$$

The **candidate clique** of vertex $u$ is

$$
\boxed{C_u=\{u\}\cup\operatorname{Madj}(u).}
$$

The later-neighbor set is a clique under a PEO, and $u$ is adjacent to
every vertex in that set. Hence every $C_u$ is a clique.

**Lemma: Every maximal clique occurs among the candidate cliques.**

**Proof:** Take an arbitrary maximal clique $K$. Let $u$ be the
earliest vertex of $K$ in the PEO. Every other vertex of $K$ is later
than $u$ and adjacent to it. Thus

$$
K\setminus\{u\}\subseteq\operatorname{Madj}(u)
\quad\Longrightarrow\quad
K\subseteq C_u.
$$

Since $C_u$ is a clique and $K$ is maximal, this inclusion cannot be
strict. Therefore

$$
\boxed{K=C_u.}
$$

### 5.2 Parent vertex and the key inclusion

When $\operatorname{Madj}(u)\ne\varnothing$, define the **parent**
$p(u)$ as the *earliest* later neighbor of $u$:

$$
p(u)=\underset{v\in\operatorname{Madj}(u)}
{\operatorname{arg\,min}}\ \operatorname{pos}(v).
$$

When the later-neighbor set is empty, the code uses **-1 as a sentinel**
rather than a vertex ID.

Fix a vertex $u$ with parent $p=p(u)$. Because
$\operatorname{Madj}(u)$ is a clique, all other later neighbors of $u$
are adjacent to $p$. As $p$ is the earliest among them, the remaining
vertices also occur later than $p$. This proves the important inclusion:

$$
\boxed{
\operatorname{Madj}(u)\setminus\{p\}
\subseteq\operatorname{Madj}(p).
}
\tag{1}
$$

### 5.3 Parent–child subsumption condition

The implemented maximality check compares the sizes of the two sets:

$$
\boxed{
|\operatorname{Madj}(u)|-1=
|\operatorname{Madj}(p)|.
}
\tag{2}
$$

**Why does this test work for the given parent–child pair?**

From equation (1), the left-hand set is *already contained* in the
right-hand set. When their sizes are equal, the two sets are identical:

$$
\operatorname{Madj}(u)\setminus\{p\}
=\operatorname{Madj}(p).
$$

Consequently,

$$
\begin{aligned}
C_p
&=\{p\}\cup\operatorname{Madj}(p)\\
&=\operatorname{Madj}(u)\\
&\subsetneq\{u\}\cup\operatorname{Madj}(u)\\
&=C_u.
\end{aligned}
\tag{3}
$$

Thus $C_p$ is **not maximal** when the size equality holds for some
child $u$.

Conversely, if $C_p\subsetneq C_u$ for this same parent–child pair,
the set containment forces the corresponding later-neighbor sets to
be equal, which gives the size equality again.

**Pairwise equivalence (for $p=p(u)$):**

$$
\boxed{
C_p\subsetneq C_u
\iff
|\operatorname{Madj}(u)|-1=|\operatorname{Madj}(p)|.
}
$$

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

$$
\boxed{S=\sum_{K\in\mathcal{K}_{\max}}|K|.}
$$

Constructing the output requires $O(S)$ time. Including hash-based
PEO verification, the implementation's expected total time is

$$
\boxed{
T_{\mathrm{extract}}(n,m,S)=O(n+m+S)
\quad\text{(expected)}.
}
$$

Space including graph and materialized output is $O(n+m+S)$.
The later-neighbor lists themselves occupy $O(n+m)$ space.

Explicitly including $S$ clarifies the cost of output construction.

## 6. Seven-vertex example and dry run

The graph in the sample data file contains seven vertices and ten edges.

$$
V=\{0,1,2,3,4,5,6\}
$$

$$
\begin{aligned}
E=\bigl\{&
\{0,1\},\{1,3\},\{0,2\},\{2,3\},\{2,4\},\\
&\{2,5\},\{4,5\},\{3,6\},\{0,3\},\{1,2\}
\bigr\}.
\end{aligned}
$$

MCS selects vertices in this order with the chosen bucket tie-breaking:

$$
6\ \longrightarrow\ 3\ \longrightarrow\ 0\ \longrightarrow\
2\ \longrightarrow\ 1\ \longrightarrow\ 5\ \longrightarrow\ 4.
$$

Because MCS fills the ordering from **right to left**, the resulting
perfect elimination ordering is the **reverse** of the selection sequence:

$$
\boxed{\alpha=(4,5,1,2,0,3,6).}
$$

The extracted maximal cliques are

$$
\boxed{
\begin{aligned}
K_1&=\{0,1,2,3\},\\
K_2&=\{2,4,5\},\\
K_3&=\{3,6\}.
\end{aligned}
}
$$

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