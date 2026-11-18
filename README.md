# MTP End-Semester — Chordal Graphs and Dynamic Clique Trees

**M.Tech CSE | IIT Bhubaneswar | End-semester presentation: November 30, 2026**

Private working repository based on Louis Ibarra (2008), *Fully Dynamic Algorithms for Chordal Graphs and Split Graphs* (DOI 10.1145/1383369.1383371). The current implementation is a **static clique-tree foundation** for the fully dynamic work.

## Project structure

- `include/mtp/` — reusable C++ interfaces for graph storage, MCS/PEO, maximal cliques, weighted clique graph, and clique tree.
- `src/` — implementations and command-line program.
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
Clique graph W_G edges (clique indices are 0-based):
K0 -- K1 [weight=1]
K1 -- K2 [weight=1]
Clique tree T edges:
K0 -- K1 [weight=1]
K1 -- K2 [weight=1]
~~~

Graph input format: first line contains the number of vertices and edges (`V E`), followed by `E` undirected pairs (`u v`) using zero-based vertex IDs. Invalid input is rejected. The graph must be chordal for maximal-clique and clique-tree extraction.

## Current implementation status

| Component | Status |
| --- | --- |
| Graph representation, MCS, and PEO verification | Implemented and tested |
| Maximal clique enumeration | Implemented and tested |
| Weighted clique intersection graph W_G | **Implemented and tested** |
| Static clique tree T | **Implemented and tested** |
| Disconnected graphs and dummy clique-tree edges | **Implemented and tested** |
| Chordality-preserving edge insertion/deletion queries | Pending |
| Incremental clique-tree updates | Pending |

## Verification

The clique graph and clique tree were tested on **19,048 chordal graphs with 1–6 vertices**, and **600 seeded random chordal graphs with 7–12 vertices**. An independent Prüfer-sequence oracle checks the maximum spanning-tree weight for every tested chordal graph with up to 5 vertices. The tests also check the induced subtree property, edge weights, dummy edges, and malformed clique input.

## Mathematical notation and report policy

The GitHub report uses inline `$...$` for short symbols and **fenced `math` blocks** for displayed equations. Avoid bare `$$` blocks, which previously rendered as literal source. Example:

```math
w(i,j)=|K_i\cap K_j|
```

All developments must update the single file `docs/MTP_REPORT.md` with: source section, definition, algorithm, correctness argument, complexity, worked example, tests, and limitations.

**Scope limitation:** Static construction of a clique tree is not yet Ibarra's fully dynamic edge-update algorithm.
