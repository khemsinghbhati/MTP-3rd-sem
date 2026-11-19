cmake_minimum_required(VERSION 3.16)
project(MTPChordalGraphs VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(mtp_graphs
    src/graph.cpp
    src/mcs.cpp
    src/maximal_cliques.cpp
    src/clique_tree_blair_peyton.cpp
    src/dynamic_ibarra.cpp
    src/indexed_dynamic_ibarra.cpp
    src/globally_biased_binary_tree.cpp
    src/worst_case_dynamic_tree.cpp
    src/dynamic_ibarra_a2.cpp
    src/fast_delete_ibarra.cpp
    src/dynamic_split_ibarra.cpp
)
target_include_directories(mtp_graphs PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_options(mtp_graphs PRIVATE -Wall -Wextra -Wpedantic)

add_executable(mtp_cli src/main.cpp)
target_link_libraries(mtp_cli PRIVATE mtp_graphs)
target_compile_options(mtp_cli PRIVATE -Wall -Wextra -Wpedantic)

add_executable(mtp_dynamic_cli src/dynamic_main.cpp)
target_link_libraries(mtp_dynamic_cli PRIVATE mtp_graphs)
target_compile_options(mtp_dynamic_cli PRIVATE -Wall -Wextra -Wpedantic)

add_executable(mtp_a2_cli src/a2_main.cpp)
target_link_libraries(mtp_a2_cli PRIVATE mtp_graphs)
target_compile_options(mtp_a2_cli PRIVATE -Wall -Wextra -Wpedantic)

add_executable(mtp_indexed_cli src/indexed_main.cpp)
target_link_libraries(mtp_indexed_cli PRIVATE mtp_graphs)
target_compile_options(mtp_indexed_cli PRIVATE -Wall -Wextra -Wpedantic)

include(CTest)
if(BUILD_TESTING)
    add_executable(mtp_tests tests/test_algorithms.cpp)
    target_link_libraries(mtp_tests PRIVATE mtp_graphs)
    target_compile_options(mtp_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME chordal_algorithms COMMAND mtp_tests)
    
    # Quadratic explicit-clique-graph implementation is an oracle/baseline,
    # not part of the production path.
    add_library(mtp_clique_baseline STATIC
        src/clique_graph.cpp
        src/clique_tree.cpp
    )
    target_link_libraries(mtp_clique_baseline PUBLIC mtp_graphs)
    target_compile_options(mtp_clique_baseline PRIVATE -Wall -Wextra -Wpedantic)

    add_executable(mtp_clique_tree_tests tests/test_clique_tree.cpp)
    target_link_libraries(mtp_clique_tree_tests PRIVATE mtp_clique_baseline)
    target_compile_options(mtp_clique_tree_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME clique_tree_algorithms COMMAND mtp_clique_tree_tests)

    add_executable(mtp_linear_clique_tree_tests tests/test_linear_clique_tree.cpp)
    target_link_libraries(mtp_linear_clique_tree_tests PRIVATE mtp_graphs)
    target_compile_options(mtp_linear_clique_tree_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME linear_clique_tree COMMAND mtp_linear_clique_tree_tests)

    add_executable(mtp_ibarra_dynamic_tests tests/test_ibarra_dynamic.cpp)
    target_link_libraries(mtp_ibarra_dynamic_tests PRIVATE mtp_graphs)
    target_compile_options(mtp_ibarra_dynamic_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME ibarra_first_dynamic_implementation COMMAND mtp_ibarra_dynamic_tests)

    add_executable(mtp_ibarra_split_tests tests/test_ibarra_split.cpp)
    target_link_libraries(mtp_ibarra_split_tests PRIVATE mtp_graphs)
    target_compile_options(mtp_ibarra_split_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME ibarra_split_graph_implementation COMMAND mtp_ibarra_split_tests)

    add_executable(mtp_ibarra_split_stats_tests tests/test_ibarra_split_stats.cpp)
    target_link_libraries(mtp_ibarra_split_stats_tests PRIVATE mtp_graphs)
    target_compile_options(mtp_ibarra_split_stats_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME ibarra_split_four_numbers COMMAND mtp_ibarra_split_stats_tests)

    add_executable(mtp_ibarra_a2_tests tests/test_ibarra_a2.cpp)
    target_link_libraries(mtp_ibarra_a2_tests PRIVATE mtp_graphs)
    target_compile_options(mtp_ibarra_a2_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME ibarra_section5_actual_ST_dynamic_tree COMMAND mtp_ibarra_a2_tests)

    add_executable(mtp_worst_case_forest_tests tests/test_worst_case_dynamic_tree.cpp)
    target_link_libraries(mtp_worst_case_forest_tests PRIVATE mtp_graphs)
    target_compile_options(mtp_worst_case_forest_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME sleator_tarjan_1983_dynamic_forest COMMAND mtp_worst_case_forest_tests)

    add_executable(mtp_globally_biased_tests tests/test_globally_biased_binary_tree.cpp)
    target_link_libraries(mtp_globally_biased_tests PRIVATE mtp_graphs)
    target_compile_options(mtp_globally_biased_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME published_globally_biased_binary_tree COMMAND mtp_globally_biased_tests)

    add_executable(mtp_ibarra_indexed_tests tests/test_ibarra_indexed.cpp)
    target_link_libraries(mtp_ibarra_indexed_tests PRIVATE mtp_graphs)
    target_compile_options(mtp_ibarra_indexed_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME ibarra_indexed_path_variant COMMAND mtp_ibarra_indexed_tests)

    add_executable(mtp_ibarra_fast_tests tests/test_ibarra_fast.cpp)
    target_link_libraries(mtp_ibarra_fast_tests PRIVATE mtp_graphs)
    target_compile_options(mtp_ibarra_fast_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME ibarra_fast_delete_query COMMAND mtp_ibarra_fast_tests)

    add_executable(mtp_ibarra_fast_stress_tests tests/test_ibarra_fast_stress.cpp)
    target_link_libraries(mtp_ibarra_fast_stress_tests PRIVATE mtp_graphs)
    target_compile_options(mtp_ibarra_fast_stress_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME ibarra_fast_large_sparse_dense COMMAND mtp_ibarra_fast_stress_tests)
    add_test(NAME ibarra_dynamic_sample_cli COMMAND mtp_dynamic_cli
             ${CMAKE_CURRENT_SOURCE_DIR}/data/example_7_vertices.txt
             ${CMAKE_CURRENT_SOURCE_DIR}/data/dynamic_example_ops.txt)
    add_test(NAME ibarra_indexed_sample_cli COMMAND mtp_indexed_cli
             ${CMAKE_CURRENT_SOURCE_DIR}/data/example_7_vertices.txt
             ${CMAKE_CURRENT_SOURCE_DIR}/data/dynamic_example_ops.txt)
    add_test(NAME ibarra_a2_sample_cli COMMAND mtp_a2_cli
             ${CMAKE_CURRENT_SOURCE_DIR}/data/example_7_vertices.txt
             ${CMAKE_CURRENT_SOURCE_DIR}/data/dynamic_example_ops.txt)
    add_test(NAME ibarra_a2_fast_delete_sample_cli COMMAND mtp_a2_cli
             ${CMAKE_CURRENT_SOURCE_DIR}/data/example_7_vertices.txt
             ${CMAKE_CURRENT_SOURCE_DIR}/data/dynamic_example_ops.txt --fast-delete)
endif()

# Optional reproducible timing experiment (not included in routine CI tests).
option(MTP_BUILD_BENCHMARKS "Build asymptotic-performance benchmark harness" OFF)
if(MTP_BUILD_BENCHMARKS)
    add_executable(mtp_benchmarks benchmarks/benchmark_dynamic.cpp)
    target_link_libraries(mtp_benchmarks PRIVATE mtp_graphs)
    target_compile_options(mtp_benchmarks PRIVATE -Wall -Wextra -Wpedantic)
    add_executable(mtp_benchmark_indexed benchmarks/benchmark_indexed.cpp)
    target_link_libraries(mtp_benchmark_indexed PRIVATE mtp_graphs)
    target_compile_options(mtp_benchmark_indexed PRIVATE -Wall -Wextra -Wpedantic)
endif()
