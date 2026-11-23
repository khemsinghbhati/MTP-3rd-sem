#!/usr/bin/env bash
# Rebuild the MTP benchmarks and regenerate the CSV files of this folder.
# Usage:  bash run_benchmarks.sh /path/to/MTP-3rd-sem [output-directory]
set -euo pipefail
REPO="$(cd "${1:?path to MTP-3rd-sem}" && pwd)"
OUT="$(mkdir -p "${2:-./benchmarks_results_$(date +%F)}" && cd "${2:-./benchmarks_results_$(date +%F)}" && pwd)"
DAY="$(date +%F)"

cmake -S "$REPO" -B "$REPO/build" -DCMAKE_BUILD_TYPE=Release -DMTP_BUILD_BENCHMARKS=ON
cmake --build "$REPO/build" -j

cd "$REPO/build"
./mtp_benchmarks              > "$OUT/a1_star_$DAY.csv"
./mtp_benchmark_a2_st         > "$OUT/a1_vs_st_a2_path_$DAY.csv"
./mtp_a2_preprocessing_audit  > "$OUT/a2_empty_preprocessing_$DAY.csv"
./mtp_a2_two_hub_stress       > "$OUT/a2_two_hub_search_$DAY.csv"
./mtp_a2_operation_audit      > "$OUT/a2_section4_insert_st_calls_$DAY.csv"
./mtp_benchmark_indexed       > "$OUT/a1_vs_indexed_path_$DAY.csv"
./mtp_a2_linear_insert_tests | grep -v '^PASS' > "$OUT/a2_linear_insert_rebuild_steps_$DAY.csv"

{
  echo "date: $(date --iso-8601=seconds)"
  echo "commit: $(git -C "$REPO" rev-parse HEAD) ($(git -C "$REPO" rev-parse --abbrev-ref HEAD))"
  echo "local changes: $(git -C "$REPO" status --short | wc -l) files"
  echo; echo "== OS";       grep -E '^(PRETTY_NAME|VERSION)=' /etc/os-release; uname -srm
  echo; echo "== CPU";      lscpu | grep -E 'Model name|^CPU\(s\)|Thread|Core|Socket|Hypervisor|Virtualization|L1d|L1i|L2|L3|Model:|Stepping'
  echo; echo "== Memory";   free -h | head -2
  echo; echo "== Toolchain"; g++ --version | head -1; cmake --version | head -1; make --version | head -1; ldd --version | head -1
  dpkg -l 2>/dev/null | awk '/libstdc\+\+6|libstdc\+\+-13-dev/ {print $2, $3}'
  echo; echo "== Flags";    grep -h '^CXX_FLAGS' "$REPO/build/CMakeFiles/mtp_graphs.dir/flags.make"
  grep -E '^CMAKE_BUILD_TYPE|^CMAKE_CXX_FLAGS(_RELEASE)?:' "$REPO/build/CMakeCache.txt"
} > "$OUT/environment_$DAY.txt"

echo "CSV files and environment written to $OUT"
