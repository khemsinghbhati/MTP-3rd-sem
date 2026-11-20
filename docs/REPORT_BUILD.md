# Final LaTeX report build

The **authoritative submission source** is
[`MTP_FINAL_REPORT.tex`](MTP_FINAL_REPORT.tex).
The PDF is generated from this LaTeX source. The report freezes
all `src/` and `include/` code at
`2bf723c2b3b57a6b532058f21742b8125292852a` and explicitly
marks A2's Section 7 preprocessing target as **not achieved**.

On Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y texlive-latex-base texlive-latex-recommended texlive-fonts-recommended
cd docs
pdflatex -interaction=nonstopmode -halt-on-error MTP_FINAL_REPORT.tex
pdflatex -interaction=nonstopmode -halt-on-error MTP_FINAL_REPORT.tex
```

Expected file: `docs/MTP_FINAL_REPORT.pdf`. The LaTeX source was
compiled twice with `pdflatex` locally during final report
preparation on 21 November 2026 (10 A4 pages; no unresolved
references or overfull boxes). GitHub Actions also builds and
uploads the PDF as a workflow artifact.

The longer `MTP_REPORT.md` is the historical detailed
development record. The final audit summary is
[`FINAL_AUDIT_2026-11-21.md`](FINAL_AUDIT_2026-11-21.md).

Independent research checks are reproducible using:

```bash
python -m pip install -r audit/requirements.txt
python audit/verify_chordal_identities.py 17
python audit/verify_w_updates.py
```

The core algorithms remain frozen; changes to them require
a new audit baseline and fresh verification, not an edit to the
existing report's claims.
