# Build the final MTP report (revised 100-page chordal-graph-only edition)

The authoritative final source is **`docs/MTP_FINAL_REPORT.tex`**. It is a standalone LaTeX document with no external figures or required `\input{}` files. The old `docs/MTP_IMPLEMENTATION_DETAILS.tex` remains in Git history and in the repository only as a companion for the previous 37-page edition; **do not include it in this new report**.

The final report covers only the chordal-graph algorithms (A1, A2, A1+ and A2+). Older reports and the complete frozen repository, including other implemented modules, are retained for traceability and are not the submission.

The first page contains the public GitHub repository link, student details, and the preserved frozen-code commit. The two signature-page dates are intentionally blank for student/supervisor review. The original A2 `O(m+n log n)` preprocessing target remains explicitly marked **not achieved**.

On Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y texlive-latex-base texlive-latex-recommended \
  texlive-latex-extra texlive-pictures texlive-science \
  texlive-fonts-recommended
cd docs
pdflatex -interaction=nonstopmode -halt-on-error MTP_FINAL_REPORT.tex
pdflatex -interaction=nonstopmode -halt-on-error MTP_FINAL_REPORT.tex
pdflatex -interaction=nonstopmode -halt-on-error MTP_FINAL_REPORT.tex
```

Output: `docs/MTP_FINAL_REPORT.pdf` (A4, 100 pages).

## Raw performance evidence

`benchmarks/results/2026-11-23/` contains the original seven CSVs used in Chapter 11, a second benchmark run, the experimental-machine description (`ENVIRONMENT.md`), a reproducible `run_benchmarks.sh` and a byte-preserved copy of the user-supplied archive. Timing values are single-run measurements or within-run per-operation means. The second-run values are useful for assessing variability but **not** a formal complexity proof. See `README_AUDIT.md`.

## Verification

The GitHub Actions `final-mtp-audit.yml` recompiles this LaTeX source, uploads a PDF artifact and verifies that `src/` and `include/` are byte-for-byte unchanged since commit `2bf723c2b3b57a6b532058f21742b8125292852a`. The separate C++ workflow runs the regression suite.

For independent mathematical audit scripts:

```bash
python -m pip install -r audit/requirements.txt
python audit/verify_chordal_identities.py 17
python audit/verify_w_updates.py
```

A signed certificate is not present; the provided certificate is a draft awaiting the supervisor's review.
