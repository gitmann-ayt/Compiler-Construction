#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace cc::ll1 {

struct Production {
    std::string lhs;
    std::vector<std::string> rhs; // empty => epsilon
};

struct Grammar {
    std::string start;
    std::unordered_set<std::string> nonterminals;
    std::unordered_set<std::string> terminals;
    std::vector<Production> prods;
};

Grammar parse_grammar_file(const std::string& path);

struct FirstFollow {
    std::unordered_map<std::string, bool> nullable;
    std::unordered_map<std::string, std::unordered_set<std::string>> first;
    std::unordered_map<std::string, std::unordered_set<std::string>> follow;
};

FirstFollow compute_first_follow(const Grammar& g);

using TableKey = std::pair<std::string, std::string>; // (nonterminal, terminal)

struct TableKeyHash {
    std::size_t operator()(const TableKey& k) const noexcept;
};

using ParseTable = std::unordered_map<TableKey, Production, TableKeyHash>;

ParseTable build_ll1_table(const Grammar& g, const FirstFollow& ff);

std::string prod_to_string(const Production& p);

} // namespace cc::ll1
