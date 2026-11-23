# Report corrections and evidence review — 23 November 2026

**Input source:** user-supplied draft `MTP_Report.tex` (approximately 105 pages) and `benchmarks_results_2026-11-23.zip`. The current authoritative final source is `docs/MTP_FINAL_REPORT.tex`.

## Applied edits

1. **Dates:** removed both predetermined **30 November 2026** dates from the certificate and declaration; supplied blank dates. Explicitly marked both pages as unsigned/draft pending review.
2. **Repository access:** retained **public** repository wording, verified against current GitHub repository visibility.
3. **Implementation scope:** changed “entire paper implemented” to specify the four core dynamic chordal variants and split-graph procedures; the optional ET-tree connectivity extension remains unimplemented and A2's published preprocessing target is unachieved.
4. **Input loading:** only the production A1 dynamic CLI uses `Graph::from_edges` (linear bulk construction). The static, A2 and indexed CLIs use individual `Graph::add_edge` calls; the report now explains their ingestion complexity separately.
5. **Matrix multiplication:** updated the outdated 2025-only exponent discussion with the October 2026 preprint bound `omega <= 9/4` **over the complex numbers**; preserved the conditional, unpublished nature of the separate Boolean-matrix-multiplication reduction.
6. **Benchmarks:** archived the full supplied ZIP, first-run CSV tables, second-run CSVs, experimental environment, and rerun script in `benchmarks/results/2026-11-23/`. Report labels datasets as user-provided; no independent reproduction on the original VM is implied. The printed benchmark values were checked against the CSVs.
7. **Comparison fairness:** identified *dense A1* specifically in the A1/A2 and A1/indexed tables and legends. The star benchmark uses *sparse A1*, which is different.
8. **Theory versus experiment:** emphasized that regression tests and slopes support observed behavior; worst-case guarantees depend on code-level analysis and the published data-structure invariants.
9. **Frozen code link:** the cover links to the immutable algorithm commit as well as the public repository.

## Source freeze

Algorithms remain unchanged at commit `2bf723c2b3b57a6b532058f21742b8125292852a`. This change only updates documentation, reference material, and archived benchmark evidence. No `src/` or `include/` files are modified.

## Compilation

Locally compiled with `pdflatex` (three passes): **105 A4 pages**, no unresolved cross-references, no overfull boxes. Some minor underfull typography warnings remain.

## Academic review before signing

The certificate/declaration are unsigned. The student and supervisor should approve the academic wording and institutional template before signing.
