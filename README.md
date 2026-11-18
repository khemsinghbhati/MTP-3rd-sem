# MTP End-Semester — Chordal Graphs and Dynamic Clique Trees

**M.Tech CSE | IIT Bhubaneswar | End-semester presentation: November 30, 2026**

Private working repository implementing Louis Ibarra's *Fully Dynamic Algorithms for Chordal Graphs and Split Graphs* (2008, DOI 10.1145/1383369.1383371). **The paper's methods and asymptotic bounds are requirements.** Static preprocessing is implemented; the four dynamic operations are not yet implemented.

## Project structure

- `include/mtp/` — modular interfaces for MCS/PEO, maximal cliques, and the direct clique-tree constructor.
- `src/` — paper-aligned production algorithms and the CLI. The quadratic clique-graph/Kruskal implementations are compiled only into the **test baseline** library.
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

## Current implementation status

| Component | Status |
| --- | --- |
| MCS and deterministic PEO verification | **$O(n+m)$; implemented and tested** |
| Maximal clique enumeration | **$O(n+m)$; implemented and tested** |
| Direct initial clique-tree construction | **$O(n+m)$; implemented and tested** |
| Explicit weighted clique graph + Kruskal | **Reference baseline only** (not production) |
| Disconnected graphs and dummy clique-tree edges | **Implemented and tested** |
| Section 3: Delete-Query / Insert-Query | Pending |
| Section 3: Delete / Insert (incremental updates) | Pending |
| Section 4: $O(n)$ per dynamic operation | Pending |
| Sections 5–6: optimized data structures and bounds | Pending |

## Verification

The direct clique-tree constructor passed **19,048 exhaustive chordal graphs** with 1–6 vertices, **800 seeded randomized chordal graphs** with 7–13 vertices, and larger sparse-star and dense-clique cases. Tests independently compare maximal cliques, tree weights, maximum spanning-tree weights, and the induced subtree property. Existing baseline tests remain. The full input parser still uses hashing for duplicate-edge validation; the $O(n+m)$ algorithmic bound assumes an already represented graph.

## Mathematical notation and report policy

The GitHub report uses inline `$...$` for short symbols and **fenced `math` blocks** for displayed equations. Avoid bare `$$` blocks, which previously rendered as literal source. Example:

```math
w(i,j)=|K_i\cap K_j|
```

All developments must update the single file `docs/MTP_REPORT.md` with: source section, definition, algorithm, correctness argument, complexity, worked example, tests, and limitations.

**Scope limitation:** The current $O(n+m)$ clique-tree construction is a direct elimination-forest contraction procedure. Ibarra cites linear-time initial construction algorithms in Section 2, but does not spell out that specific procedure. We therefore claim alignment of the **preprocessing bound and clique-tree invariants**, not verbatim pseudocode from the paper. Implementing and validating the paper's actual Section 3 update rules and Sections 4–6 data structures is the next priority.
