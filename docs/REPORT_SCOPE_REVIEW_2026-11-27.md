# Final report scope revision — 27 November 2026

The authoritative report is [MTP_FINAL_REPORT.tex](MTP_FINAL_REPORT.tex). At the supervisor's direction the final submission now covers only chordal graphs: static preprocessing, A1, A2, the faster deletion-query variants, verification, experiments and limitations.

## Review and corrections

- Compared the supplied revised LaTeX with the preceding report and checked its corresponding 100-page PDF. Removed material includes the separate graph-class chapter, background definitions, algorithm listings, related test rows, conclusions and glossary entries. The exact title of Ibarra's cited paper remains unchanged in the bibliography.
- Confirmed the report's 21 chordal-graph CTest entries against the repository's 23 registered entries. The full test suite and source remain unchanged.
- Corrected unsupported tight-bound notation to upper bounds: A2 preprocessing is O(n²+mn), with a verified quadratic initialization cost on an empty graph; a linear count of dynamic-tree calls gives an O(n log n) upper bound, not a matching lower bound on running time.
- Verified the published support for the linear biased binary-tree construction in Bent–Sleator–Tarjan (1985), §4, pp. 563–564, Fig. 15 and the extension of Theorem 12. The C++ invariant-preservation assumption remains explicit.
- Clarified that the Boolean matrix multiplication reduction decodes a yes answer as zero. Its implication is conditional, not a proven impossibility.
- Retained the AI-assistance declaration, the blank signature dates, the public repository link on the cover, the archived benchmark evidence and the unachieved A2 preprocessing target. The report's timing numbers are historical supplied measurements, not new experiments.
- Compiled the corrected report with three pdflatex passes: 100 A4 pages, no undefined references or overfull boxes; the cover and representative pages were checked.

The compiled LaTeX source Git blob is `09e40ef9ea9ad961229a22f9cd971c4524520522`. The source code under `src/` and `include/` is unchanged from frozen commit `2bf723c2b3b57a6b532058f21742b8125292852a`. This is a documentation review, not a new machine-checked proof or independent rerun of all experiments. Historical reports and experimental records are retained.
