#include "mtp/dynamic_ibarra.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
mtp::Graph load_graph(const std::string& path) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("cannot open graph file");
    int n=-1, m=-1;
    if (!(input >> n >> m) || n<0 || m<0)
        throw std::runtime_error("invalid graph header");
    mtp::Graph graph(n);
    for (int i=0;i<m;++i) {
        int u,v;
        if (!(input >> u >> v))
            throw std::runtime_error("incomplete edge list");
        if (graph.has_edge(u,v))
            throw std::runtime_error("duplicate input edge");
        graph.add_edge(u,v);
    }
    std::string tail;
    if (input >> tail)
        throw std::runtime_error("unexpected data after graph edges");
    return graph;
}
void print_tree(const mtp::CliqueTree& tree) {
    std::cout << "Maximal cliques (" << tree.cliques.size() << "):\n";
    for (std::size_t i=0;i<tree.cliques.size();++i) {
        std::cout << " K" << i << " = {";
        for (int v:tree.cliques[i])std::cout << ' ' << v;
        std::cout << " }\n";
    }
    for (const auto& e:tree.edges)
        std::cout << " K" << e.first << " -- K" << e.second
                  << " weight=" << e.weight
                  << (e.dummy ? " (dummy)" : "") << '\n';
}
}
int main(int argc,char** argv) {
    if(argc!=3) {
        std::cerr << "Usage: mtp_dynamic_cli graph.txt operations.txt\n"
                  << "Commands: Delete-Query u v | Insert-Query u v | "
                     "Delete u v | Insert u v | Show\n";
        return 1;
    }
    try {
        mtp::DynamicChordalGraph dynamic(load_graph(argv[1]));
        std::ifstream script(argv[2]);
        if(!script)throw std::runtime_error("cannot open operations file");
        std::string line;
        int line_number=0;
        while(std::getline(script,line)) {
            ++line_number;
            const auto nonwhite=line.find_first_not_of(" \t\r");
            if(nonwhite==std::string::npos||line[nonwhite]=='#')continue;
            std::istringstream input(line);
            std::string command;
            input >> command;
            if(command=="Show") {
                std::string extra;
                if(input>>extra)throw std::runtime_error("Show takes no arguments");
                print_tree(dynamic.snapshot()); // debug-only, not timed
                continue;
            }
            int u=-1,v=-1;
            std::string extra;
            if(!(input>>u>>v)||(input>>extra))
                throw std::runtime_error("invalid operation arguments at line "+
                                         std::to_string(line_number));
            bool result=false;
            if(command=="Delete-Query") result=dynamic.delete_query(u,v);
            else if(command=="Insert-Query") result=dynamic.insert_query(u,v);
            else if(command=="Delete") result=dynamic.delete_edge(u,v);
            else if(command=="Insert") result=dynamic.insert_edge(u,v);
            else throw std::runtime_error("unknown operation at line "+
                                          std::to_string(line_number));
            std::cout << command << ' ' << u << ' ' << v
                      << " => " << (result ? "yes" : "no") << '\n';
        }
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 2;
    }
    return 0;
}
