#include "ll1.h"

#include <iostream>
#include <string>
#include <vector>

namespace {

bool has_flag(const std::vector<std::string>& args, const std::string& f) {
    for (auto const& a : args) if (a == f) return true;
    return false;
}

std::string get_arg_value(const std::vector<std::string>& args, const std::string& key) {
    for (std::size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == key) return args[i + 1];
    }
    return {};
}

void print_set(const std::string& name, const std::unordered_set<std::string>& s) {
    std::cout << name << " = { ";
    bool first = true;
    for (auto const& x : s) {
        if (!first) std::cout << ", ";
        std::cout << x;
        first = false;
    }
    std::cout << " }\n";
}

} // namespace

int main(int argc, char** argv) {
    try {
        std::vector<std::string> args;
        for (int i = 1; i < argc; ++i) args.push_back(argv[i]);

        std::string path = get_arg_value(args, "--grammar");
        if (path.empty()) {
            std::cerr << "Usage: ll1_tool.exe --grammar <file> [--table]\n";
            return 2;
        }

        auto g = cc::ll1::parse_grammar_file(path);
        auto ff = cc::ll1::compute_first_follow(g);

        std::cout << "Start symbol: " << g.start << "\n\n";

        std::cout << "=== Nullable ===\n";
        for (auto const& nt : g.nonterminals) {
            std::cout << nt << ": " << (ff.nullable[nt] ? "Yes" : "No") << "\n";
        }

        std::cout << "\n=== FIRST ===\n";
        for (auto const& nt : g.nonterminals) {
            print_set("FIRST(" + nt + ")", ff.first[nt]);
        }

        std::cout << "\n=== FOLLOW ===\n";
        for (auto const& nt : g.nonterminals) {
            print_set("FOLLOW(" + nt + ")", ff.follow[nt]);
        }

        if (has_flag(args, "--table")) {
            std::cout << "\n=== LL(1) Table ===\n";
            auto table = cc::ll1::build_ll1_table(g, ff);
            for (auto const& kv : table) {
                std::cout << "M[" << kv.first.first << ", " << kv.first.second << "] = " << cc::ll1::prod_to_string(kv.second) << "\n";
            }
        }

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 1;
    }
}
