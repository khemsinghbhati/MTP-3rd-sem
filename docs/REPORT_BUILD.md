# Final LaTeX report build

The **authoritative submission** is the now **37-page comprehensive technical report**, built from the main file
[`MTP_FINAL_REPORT.tex`](MTP_FINAL_REPORT.tex) **and** its included detailed implementation chapter
[`MTP_IMPLEMENTATION_DETAILS.tex`](MTP_IMPLEMENTATION_DETAILS.tex).
The report includes a clickable GitHub repository link on its cover. Both LaTeX files must be present in the same `docs/` directory.
The PDF is generated from this LaTeX source. The report freezes
all `src/` and `include/` code at
`2bf723c2b3b57a6b532058f21742b8125292852a` and explicitly
marks A2's Section 7 preprocessing target as **not achieved**.

On Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y texlive-latex-base texlive-latex-recommended texlive-fonts-recommended texlive-pictures
cd docs
pdflatex -interaction=nonstopmode -halt-on-error MTP_FINAL_REPORT.tex
pdflatex -interaction=nonstopmode -halt-on-error MTP_FINAL_REPORT.tex
```

Expected file: `docs/MTP_FINAL_REPORT.pdf`. The expanded two-file report was compiled twice with `pdflatex` locally during final report preparation on 21 November 2026 (37 A4 pages; cross-references resolved, representative pages visually checked). GitHub Actions also builds and
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
