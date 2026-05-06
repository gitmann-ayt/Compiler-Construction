#pragma once

#include "token.h"
#include "types.h"
#include "util.h"

#include <memory>
#include <string>
#include <vector>

namespace cc {

struct Expr;
struct Stmt;

struct Node {
    SourceLocation loc{};
    virtual ~Node() = default;
};

struct Expr : Node {
    ValueType type = ValueType::Unknown; // filled by semantic analysis
    virtual ~Expr() = default;
};

struct Stmt : Node {
    virtual ~Stmt() = default;
};

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

struct NumberExpr final : Expr {
    double value = 0.0;
    bool isInt = false;
};

struct IdentExpr final : Expr {
    std::string name;
};

struct UnaryExpr final : Expr {
    TokenKind op = TokenKind::Invalid;
    ExprPtr rhs;
};

struct BinaryExpr final : Expr {
    TokenKind op = TokenKind::Invalid;
    ExprPtr lhs;
    ExprPtr rhs;
};

struct CallExpr final : Expr {
    std::string callee; // log / exp (for this project)
    std::vector<ExprPtr> args;
};

struct BlockStmt final : Stmt {
    std::vector<StmtPtr> stmts;
};

struct VarDeclStmt final : Stmt {
    ValueType declaredType = ValueType::Unknown;
    std::string name;
};

struct AssignStmt final : Stmt {
    std::string name;
    ExprPtr value;
};

struct IfStmt final : Stmt {
    ExprPtr cond;
    StmtPtr thenBranch;
    StmtPtr elseBranch; // optional
};

struct WhileStmt final : Stmt {
    ExprPtr cond;
    StmtPtr body;
};

struct ReturnStmt final : Stmt {
    ExprPtr value; // optional
};

struct Program final : Node {
    std::vector<StmtPtr> stmts;
};

std::string dump_ast(const Program& program);

} // namespace cc
