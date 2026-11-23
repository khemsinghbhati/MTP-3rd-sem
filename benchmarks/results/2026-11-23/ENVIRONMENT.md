# Experimental environment — benchmark run of 23 November 2026

These CSV files are the raw output of the benchmark programs of
https://github.com/khemsinghbhati/MTP-3rd-sem used in Chapter 12 (and Tables 7.3
and 10.2) of the MTP report. Each program prints one CSV table to standard output;
nothing was edited except that one test program's final `PASS` line was dropped
from `a2_linear_insert_rebuild_steps_2026-11-23.csv`.

## Source

| Item | Value |
|---|---|
| Repository | https://github.com/khemsinghbhati/MTP-3rd-sem |
| Commit | `651b2363c10547dc06684518d2f968c4fbc67ef2` (branch `main`, committed 21 Nov 2026 04:15 +0530) |
| Algorithm core | `src/` and `include/` identical to frozen core `2bf723c2b3b57a6b532058f21742b8125292852a` |
| Working tree | clean (no local modifications) |

## Machine

| Item | Value |
|---|---|
| Platform | Docker container on a KVM virtual machine (full virtualisation) |
| CPU | Intel(R) Xeon(R) Processor @ 2.10GHz — family 6, model 207, stepping 2 |
| vCPUs | 2 (1 socket, 2 cores, 1 thread per core), 1 NUMA node |
| Caches | L1d 96 KiB (2 instances), L1i 64 KiB (2 instances), L2 4 MiB (2 instances), L3 260 MiB |
| Memory | 7.8 GiB, no swap |
| OS | Ubuntu 24.04.5 LTS (Noble Numbat) |
| Kernel | Linux 6.18.44, x86_64, SMP PREEMPT_DYNAMIC |

## Toolchain

| Item | Value |
|---|---|
| C++ compiler | g++ 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1), `/usr/bin/c++` |
| C++ standard | C++17 (`CMAKE_CXX_STANDARD 17`, required) |
| libstdc++ | headers 13.3.0 (`libstdc++-13-dev`); runtime `libstdc++.so.6` from package 14.2.0-4ubuntu2~24.04.1 (dynamically linked) |
| glibc | 2.39 (Ubuntu GLIBC 2.39-0ubuntu8.12) |
| CMake | 3.28.3, generator "Unix Makefiles" |
| Make | GNU Make 4.3 |
| Python (audits) | Python 3.13.18, NetworkX 3.7 |

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DMTP_BUILD_BENCHMARKS=ON
cmake --build build -j
```

Effective compile flags of every target (from `build/CMakeFiles/*/flags.make`):

```
-O3 -DNDEBUG -std=c++17 -Wall -Wextra -Wpedantic
```

`CMAKE_CXX_FLAGS` was empty; no LTO, no `-march`, no PGO. Binaries built
23 Nov 2026 15:23–15:24 IST.

## Runs

The programs were run once each, one after another, from the `build/` directory on the
afternoon of 23 November 2026 (IST), after the build above:

| CSV file | Program | Workload | Report |
|---|---|---|---|
| `a1_star_2026-11-23.csv` | `./mtp_benchmarks` | sparse A1 on stars K(1,n−1); construction, then 200 insert/delete cycles of a leaf–leaf edge (mean per operation) | Table 12.3, Fig. 12.1 |
| `a1_vs_st_a2_path_2026-11-23.csv` | `./mtp_benchmark_a2_st` | dense A1 vs A2 on paths P_n; construction, then 700 repetitions of the rejected `Insert-Query(0,n−1)` | Table 12.4, Fig. 12.2 |
| `a2_empty_preprocessing_2026-11-23.csv` | `./mtp_a2_preprocessing_audit` | A2 construction on the empty graph (m = 0) | Table 10.2, Fig. 12.3 |
| `a2_two_hub_search_2026-11-23.csv` | `./mtp_a2_two_hub_stress` | double-subsumption insertion on two-hub trees, n = 2k+3, best of 256 labellings | Table 7.3 |
| `a2_section4_insert_st_calls_2026-11-23.csv` | `./mtp_a2_operation_audit` | dynamic-tree link/cut calls of A2 insertions on stars and double stars | listed in Table 12.2 (counts not tabulated) |
| `a1_vs_indexed_path_2026-11-23.csv` | `./mtp_benchmark_indexed` | dense A1 vs indexed variant on paths; 2000 repetitions of `Insert-Query(0,n−1)` | Table 12.5 |
| `a2_linear_insert_rebuild_steps_2026-11-23.csv` | `./mtp_a2_linear_insert_tests` | linear Sleator–Tarjan rebuild steps for double-subsumption insertions | Fig. 12.4 |

Exact commands:

```bash
cd build
./mtp_benchmarks              > a1_star_2026-11-23.csv
./mtp_benchmark_a2_st         > a1_vs_st_a2_path_2026-11-23.csv
./mtp_a2_preprocessing_audit  > a2_empty_preprocessing_2026-11-23.csv
./mtp_a2_two_hub_stress       > a2_two_hub_search_2026-11-23.csv
./mtp_a2_operation_audit      > a2_section4_insert_st_calls_2026-11-23.csv
./mtp_benchmark_indexed       > a1_vs_indexed_path_2026-11-23.csv
./mtp_a2_linear_insert_tests | grep -v '^PASS' > a2_linear_insert_rebuild_steps_2026-11-23.csv
```

(`run_benchmarks.sh` in this folder does exactly this and also records the environment.)

## Units and measurement method

- All `*_ms` columns are milliseconds, measured with `std::chrono::steady_clock` and stored as
  `double`. Per-operation columns are the mean over the repetitions of one run
  (200, 700 or 2000, as above); construction columns are a single measurement.
- The report converts per-operation times to microseconds (× 1000) and rounds for display.
- No CPU pinning, frequency control, warm-up runs or repeated trials were used; the machine
  is a shared virtual machine. Timing values will differ from run to run and should be read
  for their growth with n, not as absolute values.
- Count columns (`memberships`, cliques, vector and `W` entries, link/cut calls, rebuild
  steps) are deterministic. A second, complete run with `run_benchmarks.sh` at 18:03 IST on
  the same day (folder `second_run_2026-11-23_1803/`, with its own `environment_2026-11-23.txt`)
  reproduced every count column exactly, including the whole of `a2_two_hub_search`,
  `a2_section4_insert_st_calls` and `a2_linear_insert_rebuild_steps`.
- Run-to-run timing variation between the two runs: within about ±20 % for most entries;
  the smallest instances of `mtp_benchmark_a2_st` (n = 64, the first measurements after the
  program starts) varied by up to 3.4×. The growth trends discussed in the report are the same
  in both runs.

## Files in this folder

- the seven CSV files of the run used in the report (`*_2026-11-23.csv`);
- `second_run_2026-11-23_1803/`: the same seven CSV files from the second run, plus the
  environment record written by the script;
- `run_benchmarks.sh`: rebuilds the benchmarks and regenerates all CSV files and an
  environment record (`bash run_benchmarks.sh /path/to/MTP-3rd-sem [output-directory]`);
- this file.

## Provenance note

During the run used for the report the programs printed to the terminal, and their output was
recorded from there; the timing columns in these files are copied from that output. Re-running
the commands above produces files with the same columns and the same count values, and timing
values close to, but not identical with, these.
