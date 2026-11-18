#include "mtp/graph.hpp"
#include "mtp/clique_graph.hpp"
#include "mtp/clique_tree.hpp"
#include "mtp/maximal_cliques.hpp"
#include "mtp/mcs.hpp"

#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void print_usage(const char* program) {
    std::cerr << "Usage: " << program << " <path-to-graph.txt>\n"
              << "Graph format: first line 'V E', followed by E lines 'u v'.\n"
              << "Vertex IDs must be 0-based; graph must be simple and undirected.\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }
    try {
        std::ifstream input(argv[1]);
        if (!input) throw std::runtime_error("cannot open graph file");

        int vertex_count = 0;
        int edge_count = 0;
        if (!(input >> vertex_count >> edge_count) || vertex_count < 0 || edge_count < 0) {
            throw std::runtime_error("invalid graph header (V E)");
        }
        mtp::Graph graph(vertex_count);
        for (int i = 0; i < edge_count; ++i) {
            int u = 0, v = 0;
            if (!(input >> u >> v)) throw std::runtime_error("missing/invalid edge");
            if (graph.has_edge(u, v)) throw std::runtime_error("duplicate edge in input");
            graph.add_edge(u, v);
        }
        std::string extra;
        if (input >> extra) throw std::runtime_error("unexpected text after edge list");

        const auto peo = mtp::maximum_cardinality_search(graph);
        std::cout << "MCS-derived ordering:";
        for (int v : peo) std::cout << ' ' << v;
        std::cout << '\n';
        if (!mtp::is_perfect_elimination_ordering(graph, peo)) {
            std::cerr << "The graph is not chordal: MCS ordering is not a PEO.\n";
            return 2;
        }

        const auto cliques = mtp::extract_maximal_cliques(graph, peo);
        std::cout << "Maximal cliques (" << cliques.size() << "):\n";
        for (const auto& clique : cliques) {
            std::cout << "{ ";
            for (int v : clique) std::cout << v << ' ';
            std::cout << "}\n";
        }

        const auto weighted_graph = mtp::build_weighted_clique_graph(cliques);
        const auto clique_tree = mtp::build_clique_tree(weighted_graph);
        if (!mtp::verify_clique_tree(weighted_graph, clique_tree)) {
            throw std::runtime_error("clique tree failed invariant verification");
        }
        std::cout << "Clique graph W_G edges (clique indices are 0-based):\n";
        for (const auto& edge : weighted_graph.edges) {
            std::cout << "K" << edge.first << " -- K" << edge.second
                      << " [weight=" << edge.weight << "]\n";
        }
        std::cout << "Clique tree T edges:\n";
        for (const auto& edge : clique_tree.edges) {
            std::cout << "K" << edge.first << " -- K" << edge.second
                      << " [weight=" << edge.weight;
            if (edge.dummy) std::cout << ", dummy";
            std::cout << "]\n";
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
