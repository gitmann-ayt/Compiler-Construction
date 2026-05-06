#include "ll1.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace cc::ll1 {

static bool is_epsilon(const std::string& s) {
    return s == "ε" || s == "eps" || s == "epsilon";
}

static std::vector<std::string> split_ws(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> out;
    std::string tok;
    while (iss >> tok) out.push_back(tok);
    return out;
}

static std::string trim(const std::string& s) {
    std::size_t a = 0;
    while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    std::size_t b = s.size();
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

Grammar parse_grammar_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("failed to open grammar file: " + path);

    Grammar g;

    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty()) continue;
        if (line.rfind("#", 0) == 0) continue;

        auto arrowPos = line.find("->");
        if (arrowPos == std::string::npos) throw std::runtime_error("grammar parse error: missing '->' in line: " + line);

        std::string lhs = trim(line.substr(0, arrowPos));
        std::string rhsAll = trim(line.substr(arrowPos + 2));

        if (lhs.empty()) throw std::runtime_error("grammar parse error: empty LHS");
        if (g.start.empty()) g.start = lhs;
        g.nonterminals.insert(lhs);

        // split alternatives by '|'
        std::vector<std::string> alts;
        {
            std::string cur;
            for (char ch : rhsAll) {
                if (ch == '|') {
                    alts.push_back(trim(cur));
                    cur.clear();
                } else {
                    cur.push_back(ch);
                }
            }
            alts.push_back(trim(cur));
        }

        for (auto const& alt : alts) {
            Production p;
            p.lhs = lhs;
            auto toks = split_ws(alt);
            if (toks.size() == 1 && is_epsilon(toks[0])) {
                // epsilon
            } else {
                for (auto const& t : toks) {
                    if (!is_epsilon(t)) p.rhs.push_back(t);
                }
            }
            g.prods.push_back(std::move(p));
        }
    }

    // Collect terminals: symbols in RHS that are not nonterminals
    for (auto const& p : g.prods) {
        for (auto const& sym : p.rhs) {
            if (!g.nonterminals.count(sym)) g.terminals.insert(sym);
        }
    }

    return g;
}

static std::unordered_set<std::string> first_of_sequence(
    const std::vector<std::string>& seq,
    const FirstFollow& ff,
    const Grammar& g,
    bool* outNullable
) {
    std::unordered_set<std::string> result;
    bool nullable = true;

    if (seq.empty()) {
        if (outNullable) *outNullable = true;
        return result;
    }

    for (auto const& sym : seq) {
        if (g.terminals.count(sym)) {
            result.insert(sym);
            nullable = false;
            break;
        }
        // nonterminal
        auto itFirst = ff.first.find(sym);
        if (itFirst != ff.first.end()) {
            result.insert(itFirst->second.begin(), itFirst->second.end());
        }
        bool symNullable = false;
        auto itNull = ff.nullable.find(sym);
        if (itNull != ff.nullable.end()) symNullable = itNull->second;

        if (!symNullable) {
            nullable = false;
            break;
        }
    }

    if (outNullable) *outNullable = nullable;
    return result;
}

FirstFollow compute_first_follow(const Grammar& g) {
    FirstFollow ff;

    for (auto const& nt : g.nonterminals) {
        ff.nullable[nt] = false;
        ff.first[nt] = {};
        ff.follow[nt] = {};
    }

    if (!g.start.empty()) ff.follow[g.start].insert("$");

    // FIRST + nullable
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto const& p : g.prods) {
            bool rhsNullable = p.rhs.empty();
            std::unordered_set<std::string> accum;

            if (!p.rhs.empty()) {
                rhsNullable = true;
                for (auto const& sym : p.rhs) {
                    if (g.terminals.count(sym)) {
                        accum.insert(sym);
                        rhsNullable = false;
                        break;
                    }

                    // nonterminal
                    auto const& f = ff.first[sym];
                    accum.insert(f.begin(), f.end());

                    if (!ff.nullable[sym]) {
                        rhsNullable = false;
                        break;
                    }
                }
            }

            // Add FIRST items
            for (auto const& t : accum) {
                if (!ff.first[p.lhs].count(t)) {
                    ff.first[p.lhs].insert(t);
                    changed = true;
                }
            }

            if (rhsNullable && !ff.nullable[p.lhs]) {
                ff.nullable[p.lhs] = true;
                changed = true;
            }
        }
    }

    // Add explicit eps marker to FIRST sets for nullable non-terminals (Windows-friendly output)
    for (auto const& nt : g.nonterminals) {
        if (ff.nullable[nt]) ff.first[nt].insert("eps");
    }

    // FOLLOW
    changed = true;
    while (changed) {
        changed = false;
        for (auto const& p : g.prods) {
            for (std::size_t i = 0; i < p.rhs.size(); ++i) {
                const std::string& B = p.rhs[i];
                if (!g.nonterminals.count(B)) continue;

                std::vector<std::string> beta;
                for (std::size_t j = i + 1; j < p.rhs.size(); ++j) beta.push_back(p.rhs[j]);

                bool betaNullable = false;
                auto firstBeta = first_of_sequence(beta, ff, g, &betaNullable);

                for (auto const& t : firstBeta) {
                    if (t == "eps") continue;
                    if (!ff.follow[B].count(t)) {
                        ff.follow[B].insert(t);
                        changed = true;
                    }
                }

                if (beta.empty() || betaNullable) {
                    for (auto const& t : ff.follow[p.lhs]) {
                        if (!ff.follow[B].count(t)) {
                            ff.follow[B].insert(t);
                            changed = true;
                        }
                    }
                }
            }
        }
    }

    return ff;
}

std::size_t TableKeyHash::operator()(const TableKey& k) const noexcept {
    return std::hash<std::string>()(k.first) ^ (std::hash<std::string>()(k.second) << 1);
}

ParseTable build_ll1_table(const Grammar& g, const FirstFollow& ff) {
    ParseTable table;

    for (auto const& p : g.prods) {
        bool rhsNullable = p.rhs.empty();
        auto firstAlpha = first_of_sequence(p.rhs, ff, g, &rhsNullable);

        for (auto const& a : firstAlpha) {
            if (a == "eps") continue;
            TableKey key{p.lhs, a};
            table[key] = p;
        }

        if (rhsNullable) {
            for (auto const& b : ff.follow.at(p.lhs)) {
                TableKey key{p.lhs, b};
                table[key] = p;
            }
        }
    }

    return table;
}

std::string prod_to_string(const Production& p) {
    std::ostringstream out;
    out << p.lhs << " -> ";
    if (p.rhs.empty()) {
        out << "eps";
    } else {
        for (std::size_t i = 0; i < p.rhs.size(); ++i) {
            if (i) out << ' ';
            out << p.rhs[i];
        }
    }
    return out.str();
}

} // namespace cc::ll1
