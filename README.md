# MTP End-Semester — Chordal Graphs and Dynamic Clique Trees

> **CORE FROZEN (21 November 2026).** Algorithm source is fixed at
> `2bf723c2b3b57a6b532058f21742b8125292852a` and preserved under
> `freeze/core-2026-11-21`. The final academic submission is
> **[`docs/MTP_FINAL_REPORT.tex`](docs/MTP_FINAL_REPORT.tex)**, a
> **105-page standalone LaTeX report** (revised 23 November 2026). The short [final proof/correctness audit]
> (docs/FINAL_AUDIT_2026-11-21.md) is authoritative for implementation
> guarantees. The original A2 Section 7 `O(m+n log n)`
> preprocessing bound is **NOT achieved**; the code takes
> `O(n^2+mn)` preprocessing. The long development log is historical.


**M.Tech CSE | IIT Bhubaneswar | Third-semester MTP**

Public GitHub repository implementing Louis Ibarra's *Fully Dynamic Algorithms for Chordal Graphs and Split Graphs* (2008, DOI 10.1145/1383369.1383371). **The paper's methods and asymptotic bounds are requirements.** Blair–Peyton clique-tree preprocessing and a **sparse A1 implementation with O(n+m) preprocessing and O(n) worst-case dynamic operations** are implemented; the optional fast deletion-query index and split-graph algorithm are also available. Section 5's worst-case Sleator–Tarjan dynamic tree is implemented and tested; **the source-backed O(n) insertion repair is proved in Section 15**, while the paper's stronger preprocessing-table claim remains inconsistent with its explicitly dense W representation.

## Project structure

- `include/mtp/` — MCS/PEO, Blair–Peyton clique tree, Ibarra dynamic chordal queries/updates, modified-Yellin fast index, and split-graph interfaces.
- `src/` — published-source-aligned algorithms and CLIs. The quadratic clique-graph/Kruskal implementations are compiled only into the **test baseline** library.
- `tests/` — exhaustive, randomized, and independent small-graph verification.
- `benchmarks/results/2026-11-23/` — two archived benchmark runs, compiler/environment record, reproducible commands, and original uploaded ZIP.
- `data/` — example input graphs.
- `docs/MTP_FINAL_REPORT.tex` — **authoritative 105-page comprehensive report** (single-file LaTeX master), with a clickable GitHub link on its first page.
- `docs/MTP_IMPLEMENTATION_DETAILS.tex` — archived companion to the previous 37-page edition (no longer required for the current report); source-grounded explanation of each published algorithm and each major C++ implementation decision, including pseudocode, worked updates, ST83 internals, invariants, complexity and tests.
- `docs/REPORT_BUILD.md` — reproducible standalone LaTeX build instructions.
- `docs/FINAL_AUDIT_2026-11-21.md` — frozen proof, complexity, correctness and provenance audit.
- `docs/MTP_REPORT.md` — dated **historical development log**, not the final summary.
- `audit/` — independently reproducible chordal-graph, matrix-update and BMM-reduction checks.
- `docs/A1_LINEAR_PREPROCESSING.md` — **sparse A1 implementation, exact operations, proof and regression evidence**.
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
./build/mtp_indexed_cli data/example_7_vertices.txt data/dynamic_example_ops.txt
./build/mtp_indexed_cli data/example_7_vertices.txt data/dynamic_example_ops.txt --fast-delete
python3 scripts/check_markdown_math.py
~~~

The GitHub repository is public and can be cloned without authentication.

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
./build/mtp_benchmark_indexed > indexed_results.csv
~~~

The benchmark measures initialization, query, insertion, and deletion
timings on repeatable star graphs. One recorded measurement is in
`benchmarks/results/a1_star_2026-11-19.csv`. Timings vary with hardware,
cache, and scheduling; they are **empirical observations, not proofs** of
worst-case time bounds. Benchmark results must be compared against the
published bounds and the actual data structures used.

**A1 research conclusion:** The default `mtp_dynamic_cli` uses
`SparseIbarraChordalGraphA1`, which stores sparse clique/vertex
incidence with cross-pointers and builds its graph from a bucket-ordered
edge list. Constructor O(n+m) worst-case and all four A1
operations O(n) worst-case, while the optional A1+ index retains
its own additional cost. See `docs/A1_LINEAR_PREPROCESSING.md`.

**A2 limitation (unchanged):** A2's ST83 dynamic tree and
BST85 source-backed O(n) insertion repair are retained; the
original paper's Section 7 O(m+n log n) preprocessing claim
is NOT achieved for the dense W representation and must
not be claimed. The experimental indexed heavy-path
implementation is a separate comparison, not the certified
ST83 tree.

## Current implementation status

| Component | Status |
| --- | --- |
| MCS and deterministic PEO verification | **$O(n+m)$; implemented and tested** |
| Maximal clique enumeration | **$O(n+m)$; implemented and tested** |
| Blair–Peyton Fig. 4.2 initial clique-tree construction | **$O(n+m)$; implemented and independently tested** |
| Explicit weighted clique graph + Kruskal | **Reference baseline only** (not production) |
| Disconnected graphs and dummy clique-tree edges | **Implemented and tested** |
| Section 3: Delete-Query / Insert-Query | **Sparse A1**, worst-case $O(n)$ each |
| Section 3: Delete / Insert (incremental updates) | **Sparse A1**, worst-case $O(n)$ each |
| Section 4: $O(n)$ per dynamic operation | **Implemented**, independent tests and CI passed |
| Section 5 experimental indexed path query | Implemented; O(log² n) Insert-Query, but uses full O(n) heavy-path rebuilds instead of worst-case dynamic-tree primitives |
| Section 5: ST83 globally biased dynamic tree | Implemented with BST85-based source-supported insertion refinement; code-to-proof audit remains | 
| Section 6: modified-Yellin fast deletion query | Implemented; $O(\sqrt m)$ query and $O(m+n)$ updates | 
| Section 8: dynamic split graph + clique/chromatic/independence/cover numbers | Implemented; $O(1)$ queries and valid updates |

## Verification

The Blair–Peyton constructor passed **19,048 exhaustive nonempty chordal graphs** with 1–6 vertices, **800 seeded randomized chordal graphs** with 7–13 vertices, and larger sparse-star and dense-clique cases. Tests independently compare maximal cliques, tree weights, maximum spanning-tree weights, and the induced subtree property. The split-graph tests also independently check all four optimization numbers against brute-force subset enumeration on small graphs. Existing baseline tests remain. A1's dynamic CLI loads edges using `Graph::from_edges` with bucket-ordered sorted set construction in deterministic $O(n+m)$; legacy `Graph::add_edge` remains available for incremental updates.

## A2/A2+ published-source insertion repair — November 20, 2026

**Worst-case A2 Insert: O(n) in the revised research
implementation.** Source-based proof in
[Section 15 of the technical report](
docs/MTP_REPORT.md#15-published-source-on-a2-insertion-repair--november-20-2026).

The corrected implementation uses **the same published
Sleator–Tarjan (1983) worst-case globally biased
dynamic forest**. When one inserted graph edge
causes both neighboring maximal cliques to be
subsumed, per-neighbor ST link/cut updates
are replaced by a fresh construction of that
**same ST forest**, using:

1. ST83 Sections 4–5: size-based partition and globally
   biased binary trees for paths and dashed path sets.
2. Bent–Sleator–Tarjan (1985), Section 4, Figure 15 and
   Theorem 12: actual O(n) globally biased tree construction.
3. Counting-sort dashed-path cardinalities in [1,n]
   and O(1) machine-word leaf-rank computations.

Across all paths and path sets, each vertex appears
O(1) times, so the complete rebuilding costs
O(n). This is a **source-supported maintenance-schedule
refinement** that preserves Ibarra's clique-tree transformation
and ST's exact data structure; it is not a splay/HLD/top-tree
substitute. Ibarra does not literally specify this
rebuilding schedule, and the report distinguishes
the new composition from verbatim Ibarra pseudocode.

The graph membership lookup is deterministic
O(log n) worst-case via std::set.

**Separate unresolved source discrepancy:** Ibarra's
Section 7 preprocessing summary cannot hold for our
literal dense n-by-n W matrix and n-length
characteristic-vector materialization on an empty graph.
This limitation remains explicitly documented
in Section 14.5 of the report; O(n) **insertion**
does not magically imply linear **initialization**.

Source-focused tests: tests/test_bst85_linear_builder.cpp,
tests/test_linear_st_forest.cpp,
tests/test_a2_linear_insertion.cpp, plus
tests/test_ibarra_a2.cpp and sanitizer workflows.

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
`docs/MTP_REPORT.md`. The Section 5 dynamic tree is implemented, with the separate final proof-level audit recorded in the report.

## Mathematical notation and report policy

The GitHub report uses inline `$...$` for short symbols and **fenced `math` blocks** for displayed equations. Avoid bare `$$` blocks, which previously rendered as literal source. Example:

```math
w(i,j)=|K_i\cap K_j|
```

All developments must update the single file `docs/MTP_REPORT.md` with: source section, definition, algorithm, correctness argument, complexity, worked example, tests, and limitations.

**Scope and complexity limitation:** The production A1 CLI and
`SparseIbarraChordalGraphA1` now initialize in O(n+m) from a
bulk-loaded graph and perform all four operations in O(n) worst-case.
The previous dense `DynamicChordalGraph` remains in the codebase as
the internal representation supporting A2 and indexed comparison
modules. A2's **Section 7 preprocessing bound remains unachieved**;
its actual cost, source discrepancy and corrected R1 research
alternative are documented in the report and draft PRs.
