#pragma once

#include "ast.h"
#include "types.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace cc {

struct Symbol {
    ValueType type = ValueType::Unknown;
};

class SymbolTable {
public:
    void push_scope();
    void pop_scope();

    bool declare(const std::string& name, ValueType type);
    const Symbol* lookup(const std::string& name) const;

private:
    std::vector<std::unordered_map<std::string, Symbol>> scopes_;
};

class SemanticAnalyzer {
public:
    void analyze(Program& program);

private:
    void analyze_stmt(Stmt& stmt);
    ValueType analyze_expr(Expr& expr);

    SymbolTable sym_;
};

} // namespace cc
