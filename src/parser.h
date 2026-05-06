#pragma once

#include "ast.h"
#include "lexer.h"

#include <memory>
#include <string>

namespace cc {

class Parser {
public:
    explicit Parser(Lexer lexer);

    std::unique_ptr<Program> parse_program();

private:
    const Token& peek() { return lexer_.peek(); }
    Token consume() { return lexer_.next(); }

    bool accept(TokenKind kind);
    Token expect(TokenKind kind, const std::string& message);

    StmtPtr parse_stmt();
    StmtPtr parse_block_like();

    StmtPtr parse_vardecl();
    StmtPtr parse_assign_or_expr_stmt();
    StmtPtr parse_if();
    StmtPtr parse_while();
    StmtPtr parse_return();

    ExprPtr parse_expr(int minPrec = 0);
    ExprPtr parse_unary();
    ExprPtr parse_primary();

    int precedence(TokenKind op) const;
    bool is_right_associative(TokenKind op) const;
    bool is_binary_op(TokenKind op) const;

    Lexer lexer_;
};

} // namespace cc
