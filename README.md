# MTP End-Semester — Chordal Graphs and Dynamic Clique Trees

**M.Tech CSE | IIT Bhubaneswar | End-semester presentation: November 30, 2026**

Private working repository implementing Louis Ibarra's *Fully Dynamic Algorithms for Chordal Graphs and Split Graphs* (2008, DOI 10.1145/1383369.1383371). **The paper's methods and asymptotic bounds are requirements.** Blair–Peyton clique-tree preprocessing, Ibarra's first four dynamic chordal operations, the optional fast deletion-query index, and his constant-time split-graph algorithm are implemented. Section 5's dynamic-tree optimization remains unfinished.

## Project structure

- `include/mtp/` — MCS/PEO, Blair–Peyton clique tree, Ibarra dynamic chordal queries/updates, modified-Yellin fast index, and split-graph interfaces.
- `src/` — published-source-aligned algorithms and CLIs. The quadratic clique-graph/Kruskal implementations are compiled only into the **test baseline** library.
- `tests/` — exhaustive, randomized, and independent small-graph verification.
- `data/` — example input graphs.
- `docs/MTP_REPORT.md` — **one evolving detailed technical report**.
- `scripts/check_markdown_math.py` — structural check for GitHub's math rendering conventions.
- `.github/workflows/ci.yml` — automated build and test workflow.

## Build and run (Ubuntu)

Requires C++17 and CMake 3.16+.

~~~bash
sudo apt update && sudo apt install -y git g++ cmake
git clone https://github.com/khemsinghbhati/MTP-3rd-sem.git
cd MTP-3rd-sem
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/mtp_cli data/example_7_vertices.txt
./build/mtp_dynamic_cli data/example_7_vertices.txt data/dynamic_example_ops.txt
./build/mtp_dynamic_cli data/example_7_vertices.txt data/dynamic_example_ops.txt --fast-delete
python3 scripts/check_markdown_math.py
~~~

Private repository cloning requires GitHub authentication.

**Expected example output:**

~~~text
MCS-derived ordering: 4 5 1 2 0 3 6
Maximal cliques (3):
{ 4 2 5 }
{ 1 0 3 2 }
{ 3 6 }
Clique tree T edges:
K1 -- K2 [weight=1]
K0 -- K1 [weight=1]
~~~

Graph input format: first line contains the number of vertices and edges (`V E`), followed by `E` undirected pairs (`u v`) using zero-based vertex IDs. Invalid input is rejected. The graph must be chordal for maximal-clique and clique-tree extraction.

## Reproducible performance measurement

To build the optional timing harness (separate from correctness tests):

~~~bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DMTP_BUILD_BENCHMARKS=ON
cmake --build build -j
./build/mtp_benchmarks > results.csv
~~~

The benchmark measures initialization, query, insertion, and deletion
timings on repeatable star graphs. One recorded measurement is in
`benchmarks/results/a1_star_2026-11-19.csv`. Timings vary with hardware,
cache, and scheduling; they are **empirical observations, not proofs** of
worst-case time bounds. Benchmark results must be compared against the
published bounds and the actual data structures used.

**Research limitation:** Section 5's worst-case logarithmic dynamic tree
remains absent. Source inspection also found potentially inconsistent
A2 preprocessing accounting between Ibarra Sections 5.1 and 7,
documented in the single report rather than silently resolved.

## Current implementation status

| Component | Status |
| --- | --- |
| MCS and deterministic PEO verification | **$O(n+m)$; implemented and tested** |
| Maximal clique enumeration | **$O(n+m)$; implemented and tested** |
| Blair–Peyton Fig. 4.2 initial clique-tree construction | **$O(n+m)$; implemented and independently tested** |
| Explicit weighted clique graph + Kruskal | **Reference baseline only** (not production) |
| Disconnected graphs and dummy clique-tree edges | **Implemented and tested** |
| Section 3: Delete-Query / Insert-Query | Implemented, $O(n)$ each (first version) |
| Section 3: Delete / Insert (incremental updates) | Implemented, $O(n)$ each (first version) |
| Section 4: $O(n)$ per dynamic operation | Implemented and tested |
| Section 5: worst-case dynamic tree / $O(\log^2 n)$ insertion query | **Pending** | 
| Section 6: modified-Yellin fast deletion query | Implemented; $O(\sqrt m)$ query and $O(m+n)$ updates | 
| Section 8: dynamic split graph + clique/chromatic/independence/cover numbers | Implemented; $O(1)$ queries and valid updates |

## Verification

The Blair–Peyton constructor passed **19,048 exhaustive nonempty chordal graphs** with 1–6 vertices, **800 seeded randomized chordal graphs** with 7–13 vertices, and larger sparse-star and dense-clique cases. Tests independently compare maximal cliques, tree weights, maximum spanning-tree weights, and the induced subtree property. The split-graph tests also independently check all four optimization numbers against brute-force subset enumeration on small graphs. Existing baseline tests remain. The full input parser still uses hashing for duplicate-edge validation; the $O(n+m)$ algorithmic bound assumes an already represented graph.

## Research provenance rule (mandatory)

**No original algorithm design is permitted in this implementation project.**
Every algorithm, decision criterion, data-structure optimization, and
asymptotic complexity claim must be traceable to:

1. An explicitly identified section, theorem, algorithm, or pseudocode
   in **Ibarra (2008)**; or
2. A **verified, published research reference** that states the required
   method and justifies its correctness and complexity.

Each implemented module must document its exact reference, page/section,
algorithm-to-code mapping, assumptions, complexity, and verification tests.
Passing tests and matching a complexity bound are **not** enough to establish
source fidelity. If a method lacks a verified reference, it is a
**provisional research prototype**, not an accepted paper-faithful solution.
An uncited implementation must not silently be rebranded as published work.
Keep academic references in the report bibliography; do not recreate a
separate reference/archive folder.

**Candidate verified sources for linear-time clique-tree preprocessing:**

- Blair and Peyton, *An Introduction to Chordal Graphs and Clique Trees*,
  Springer (1993), Section 4.2 and **Figure 4.2**: expanded MCS obtains
  maximal cliques and clique-tree edges.
  [Primary report](https://people.math.binghamton.edu/zaslav/Oldcourses/580.S13/blair-peyton.chordal-graphs-clique-trees.ornl1992.pdf).
- Habib, McConnell, Paul, and Viennot, *Lex-BFS and Partition Refinement*,
  **Theoretical Computer Science** 234 (2000), Section 3.1,
  **Algorithm 4**: elimination-order/parent-based clique-tree construction,
  with an explicit O(n+m) bound.
  [Published article](https://doi.org/10.1016/S0304-3975(97)00241-7).

**Source correction completed:** Production uses Blair–Peyton Figure 4.2
via `src/clique_tree_blair_peyton.cpp`; the earlier provisional
elimination-forest contraction is not used in production. Ibarra
Sections 3–4 and 6 and 8 are explicitly mapped to code and tests in
`docs/MTP_REPORT.md`. The Section 5 optimized dynamic tree is **not yet implemented**.

## Mathematical notation and report policy

The GitHub report uses inline `$...$` for short symbols and **fenced `math` blocks** for displayed equations. Avoid bare `$$` blocks, which previously rendered as literal source. Example:

```math
w(i,j)=|K_i\cap K_j|
```

All developments must update the single file `docs/MTP_REPORT.md` with: source section, definition, algorithm, correctness argument, complexity, worked example, tests, and limitations.

**Scope and complexity limitation:** The published Blair–Peyton clique tree itself is built in $O(n+m)$ time. The concrete dynamic-class constructor also allocates an n-bit-like (byte-vector) characteristic vector for each maximal clique, costing $\Theta(nk)$ for k cliques; it cannot honestly claim end-to-end linear initialization on sparse stars. The first four update operations are $O(n)$ each; the Section 6 optional fast index provides $O(\sqrt m)$ deletion queries. Completing the paper's Section 5 **worst-case** dynamic-tree structure and comparing empirical scaling remain mandatory.
