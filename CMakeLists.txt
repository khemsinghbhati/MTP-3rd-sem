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
)
target_include_directories(mtp_graphs PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_options(mtp_graphs PRIVATE -Wall -Wextra -Wpedantic)

add_executable(mtp_cli src/main.cpp)
target_link_libraries(mtp_cli PRIVATE mtp_graphs)
target_compile_options(mtp_cli PRIVATE -Wall -Wextra -Wpedantic)

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
endif()
