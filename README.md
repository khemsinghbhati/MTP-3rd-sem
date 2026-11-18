# MTP End-Semester — Chordal Graphs and Dynamic Clique Trees

**M.Tech Thesis Project | IIT Bhubaneswar | End-semester presentation: November 30, 2026**

Private working repository for the chordal-graph thesis based on Louis Ibarra's paper *Fully Dynamic Algorithms for Chordal Graphs and Split Graphs* (2008).

## Repository layout

- `include/mtp/`: public interfaces for the modular C++ algorithms.
- `src/`: graph representation, MCS and PEO verification, maximal-clique extraction, CLI.
- `tests/`: exhaustive and randomized correctness tests.
- `data/`: seven-vertex example and non-chordal test.
- `docs/MTP_REPORT.md`: **one authoritative evolving detailed report**.
- `.github/workflows/`: GitHub Actions build and tests.

## Build and run (Ubuntu)

C++17 and CMake 3.16 or newer are required:

~~~bash
sudo apt update
sudo apt install -y git g++ cmake
git clone https://github.com/khemsinghbhati/MTP-3rd-sem.git
cd MTP-3rd-sem
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/mtp_cli data/example_7_vertices.txt
ctest --test-dir build --output-on-failure
~~~

Private repository access requires GitHub authentication.

Expected program output:

~~~text
MCS-derived ordering: 4 5 1 2 0 3 6
Maximal cliques (3):
{ 4 2 5 }
{ 1 0 3 2 }
{ 3 6 }
~~~

Graph input format: the first line is `V E`, followed by `E` undirected edges `u v`, using vertex IDs from 0 to $V-1$. The CLI rejects duplicate edges, invalid vertex IDs, self-loops, and malformed files. Disconnected graphs are supported; non-chordal graphs are rejected before clique extraction.

## Current progress

| Component | Status |
| --- | --- |
| Graph representation | Implemented |
| Maximum Cardinality Search (MCS) | Implemented and tested |
| Perfect Elimination Ordering (PEO) verification | Implemented and tested |
| Maximal clique extraction | Implemented and tested |
| Clique graph | Not yet implemented |
| Clique tree | Not yet implemented |
| Edge insertion/deletion queries | Not yet implemented |
| Dynamic clique tree maintenance | Not yet implemented |

The test suite covers **all simple graphs on $1\le n\le6$ vertices**, plus **600 deterministic random chordal graphs with $7\le n\le12$ vertices**, using independent small-graph oracles.

## Mathematical notation

The technical report uses GitHub's built-in LaTeX mathematics renderer.
Definitions, theorems, and derivations use centered display equations,
while shorter mathematical symbols appear inline. For example,

```math
G=(V,E),\qquad T_{\mathrm{MCS}}(n,m)=O(n+m).
```

When extending the report, use inline dollar delimiters for brief
mathematical expressions and a pair of double-dollar lines for displayed
equations. Keep actual C++ variable names, commands, and pseudocode
in code formatting rather than math notation. GitHub rejects some operator-name
macros; use `\mathrm{pos}` and `\mathrm{Madj}` for names in equations.

## Report policy

`docs/MTP_REPORT.md` is the single evolving end-semester thesis report. For each new module, update the relevant sections with theory, proof or rationale, time complexity, tests, results, and limitations. Keep the report aligned with the tested implementation.

**This repository does not yet implement the dynamic algorithms of the target paper.**

Reference: Louis Ibarra (2008), DOI: 10.1145/1383369.1383371.
