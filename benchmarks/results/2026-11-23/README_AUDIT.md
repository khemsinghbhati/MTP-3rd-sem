# November 23 benchmark evidence

These files were supplied by the project author on November 23, 2026.

- Seven first-run CSVs are transcribed from benchmark program terminal outputs;
  the `ENVIRONMENT.md` explicitly describes their provenance.
- A second-run folder contains seven more CSVs and a machine environment dump.
- Deterministic count columns have been compared across runs; timings vary.
- The original user ZIP is preserved unchanged for traceability.
- Commands and compiler settings: see `ENVIRONMENT.md` and `run_benchmarks.sh`.
- The report uses the first-run CSVs. No independent reproduction on the
  original virtual machine is claimed.
- Baseline A1 in `a1_vs_st_a2_path` and `a1_vs_indexed_path` is
  **dense** `DynamicChordalGraph`, whereas `a1_star` uses the
  production **sparse** `SparseIbarraChordalGraphA1`.
- Code benchmarked at commit `651b2363c10547dc06684518d2f968c4fbc67ef2`,
  with algorithmic `src/` and `include/` frozen at
  `2bf723c2b3b57a6b532058f21742b8125292852a`.
