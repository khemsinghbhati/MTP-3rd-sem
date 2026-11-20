# Final MTP proof and correctness audit (21 November 2026)

> **Authoritative audit for final submission.** The long `MTP_REPORT.md`
> contains dated development notes, including superseded limitations.
> The polished submission is `docs/MTP_FINAL_REPORT.tex` (compiled locally
> using `pdflatex`, two passes). The algorithm source remains **frozen**.

## Freeze and reproducibility

- **Frozen core SHA:** `2bf723c2b3b57a6b532058f21742b8125292852a`.
- Preserved reference: `freeze/core-2026-11-21`.
- Audited source files: `src/` and `include/` at that exact commit;
  the final documentation PR is restricted to `docs/`, `audit/`,
  README and workflow files. **No core implementation changes.**
- System: C++17, deterministic `std::set` adjacency lookup, input
  `Graph::from_edges` for the A1 linear-time loading guarantee.
- Existing complete CTest suite: **23/23 passed**, with final
  [run 44811326753](https://github.com/khemsinghbhati/MTP-3rd-sem/actions/runs/44811326753).
- Final A1 ASan/UBSan:
  [run 44811326840](https://github.com/khemsinghbhati/MTP-3rd-sem/actions/runs/44811326840).
- ST83/BST85 source-focused test suite:
  [run 44811326833](https://github.com/khemsinghbhati/MTP-3rd-sem/actions/runs/44811326833).
- Additional independent audit scripts are included in `audit/`.
  `python audit/verify_chordal_identities.py 17` checked 406 graphs,
  4,466 insertion queries, 150 random BMM instances and 1,776
  reduction queries; `python audit/verify_w_updates.py` checked
  1,171 valid insertions and 5,895 valid deletion formulas.
  Both passed locally using NetworkX 3.6.1.

## Source-to-code audit matrix

| Module | Correctness criterion or data structure | Frozen code | Evidence | Disposition |
| --- | --- | --- | --- | --- |
| MCS/PEO | Tarjan--Yannakakis: bucket MCS and later-neighbor clique test | `src/mcs.cpp` | `tests/test_algorithms.cpp` | Verified/tested, O(n+m) |
| Clique tree | Blair--Peyton Fig. 4.2, running-intersection, weights, maximal cliques | `src/clique_tree_blair_peyton.cpp` | `tests/test_linear_clique_tree.cpp` | Verified/tested, O(n+m) |
| A1 | Ibarra §§3–4 deletion iff unique maximal clique; insertion path-minimum **equals** exact endpoint intersection | `src/sparse_ibarra_a1.cpp` | exhaustive n≤5, randomized edits, `tests/test_sparse_a1_stress.cpp` | O(n+m) init; four O(n) operations |
| A1+ | Ibarra §§6.2–6.3 modified-Yellin member/overflow index | `src/fast_delete_ibarra.cpp` | sparse fast-index oracle, stress | Extra matrix cost; O(sqrt(m)+1) Delete-Query |
| A2 | Ibarra §§5–5.1 explicit W; ST83 worst-case globally biased forest | `src/dynamic_ibarra_a2.cpp`, `src/worst_case_dynamic_tree.cpp` | A2 exhaustive/random, ST forest invariants | O(log²n) Insert-Query, O(nlogn) Delete (source bound) |
| A2 Insert | Ibarra tree transformation plus BST85 linear construction of the **same** ST83 forest for double-subsumption | `src/dynamic_ibarra_a2.cpp` and `src/globally_biased_binary_tree.cpp` | linear-build focused tests and branch proof §15 | O(n) source-backed composition; not verbatim Ibarra schedule |
| A2 preprocessing | Ibarra §5.1 replay from n singleton cliques, dense W/characteristic arrays | `src/dynamic_ibarra_a2.cpp`, `src/dynamic_ibarra.cpp` | empty graph asymptotic witness; construction path inspection | **NOT** O(m+nlogn) as Section 7 claims; actual O(n²+mn) |
| A2+ | optional modified-Yellin index on A2 | `src/fast_delete_ibarra.cpp` | A2+ tests | Extra O(n²+mn) preprocessing envelope |
| Split graph | Hammer--Simeone degree identity; list-class updates | `src/dynamic_split_ibarra.cpp` | split graph and brute-force statistics tests | O(1) valid updates/recognition; valid edge-notification precondition |

## Proof summaries

**A1 sparse preprocessing.** For each maximal clique K, its earliest
vertex in a PEO has K = {v} ∪ later-neighbors(v), and distinct maximal
cliques have distinct earliest PEO vertices. Consequently
`sum_K |K| <= n+m`. The implementation maintains an actual list of
members for each clique and per-vertex inverse linked lists with
iterator cross-pointers, without initializing one n-vector per clique.
Building these lists and the Blair--Peyton tree costs O(n+m).
Bulk graph loading sorts by *neighbor ID buckets* so that ordered
`std::set` range construction is linear on sorted ranges.

**A1 operations.** Multi-source clique-tree BFS, marking incident
clique nodes and marking exact intersection candidates all touch at
most O(n) clique nodes/members. A valid update changes at most two
old maximal-clique representations and O(n) clique-tree links/cuts,
each O(1) for linked tree adjacency. Graph adjacency modification
costs O(n) worst-case. All four operations are O(n) worst case.

**A2 query/update.** The ST83 published globally biased-tree bound is
O(log n) worst case per tree primitive, conditional on maintaining its
invariants in the concrete code. The nearest-clique binary search
uses O(log n) primitives, and W gives O(1) final intersection access:
O(log² n) Insert-Query. Delete issues O(n) forest primitives:
O(n log n). For double clique absorption, the source-backed BST85
static builder reconstructs the unchanged ST83 representation in
O(n), giving a conditional proof of O(n) insertion together with
O(n) matrix work. **This is not a mechanically certified C++ proof;
the available source-to-code analysis and adversarial tests should
not be misrepresented as machine verification.**

**A2 preprocessing discrepancy.** The frozen constructor eagerly
initializes n singleton characteristic vectors and the
(n+2)×(n+2) W matrix (Omega(n²) even for m=0), then replays m
insertions, giving O(n²+mn) upper bound. Ibarra §5.1 explicitly
describes O(n³) direct construction or O(mn) replay,
while §7 lists O(m+nlogn); the latter is **not achieved**.
Draft R1 PR #3 is not merged. The separate audit BMM reduction is
conditional and not a published lower bound.

**Matrix versus path weights.** For maximal cliques
`{a,b,x}`, `{a,b,c}`, `{b,c,y}` on a path, each adjacent
intersection weight is 2 but the two endpoints intersect in 1
vertex. An LCA path minimum cannot substitute for W.

**Split-graph precondition.** Degree-only updates assume valid edge
insertions/deletions because the statistic object does not store
the current edge set.

## Verification classification

- **Functional correctness:** source-prescribed criteria verified by
  independent finite graph/PEO oracles, randomized dynamic tests,
  and differential checks.
- **Asymptotic claims:** A1's new representation bounds are
  accompanied by per-loop worst-case accounting. A2 operations
  depend on the published ST83/BST85 invariant guarantees and their
  implementation-specific mapping.
- **Limitations:** A2 preprocessing as published is **not achieved**.
  There is no formal proof assistant or exhaustive proof for all
  possible native pointers or machine states.
- **Research provenance:** The BMM reduction comes from the
  independent research audit furnished by the user and is
  **not attributed to a peer-reviewed source**. It proves that
  meeting both arbitrary-input A2 preprocessing and query bounds
  would imply near-quadratic BMM, not an impossibility theorem.

## Compile the final report

```bash
sudo apt update
sudo apt install -y texlive-latex-base texlive-latex-recommended texlive-fonts-recommended
cd docs
pdflatex -interaction=nonstopmode -halt-on-error MTP_FINAL_REPORT.tex
pdflatex -interaction=nonstopmode -halt-on-error MTP_FINAL_REPORT.tex
```

The main `docs/MTP_REPORT.md` stays as the **dated development
notebook**, not the authoritative clean end-semester submission.
