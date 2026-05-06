#include "parser.h"

#include "util.h"

#include <utility>

namespace cc {

Parser::Parser(Lexer lexer) : lexer_(std::move(lexer)) {}

bool Parser::accept(TokenKind kind) {
    if (peek().kind == kind) {
        consume();
        return true;
    }
    return false;
}

Token Parser::expect(TokenKind kind, const std::string& message) {
    Token t = consume();
    if (t.kind != kind) {
        fail(to_string(t.loc) + ": " + message + " (got " + token_kind_name(t.kind) + ")");
    }
    return t;
}

std::unique_ptr<Program> Parser::parse_program() {
    auto program = std::make_unique<Program>();
    program->loc = peek().loc;

    while (peek().kind != TokenKind::End) {
        if (peek().kind == TokenKind::Invalid) {
            auto bad = consume();
            fail(to_string(bad.loc) + ": unrecognised character/token '" + bad.lexeme + "'");
        }
        program->stmts.push_back(parse_stmt());
    }
    return program;
}

StmtPtr Parser::parse_stmt() {
    switch (peek().kind) {
    case TokenKind::KwInt:
    case TokenKind::KwFloat:
        return parse_vardecl();
    case TokenKind::KwIf:
        return parse_if();
    case TokenKind::KwWhile:
        return parse_while();
    case TokenKind::KwReturn:
        return parse_return();
    case TokenKind::LBrace:
    case TokenKind::KwBegin:
        return parse_block_like();
    case TokenKind::Identifier:
        return parse_assign_or_expr_stmt();
    default: {
        Token t = consume();
        fail(to_string(t.loc) + ": unexpected token in statement: " + std::string(token_kind_name(t.kind)));
    }
    }
}

StmtPtr Parser::parse_block_like() {
    if (accept(TokenKind::LBrace)) {
        auto block = std::make_unique<BlockStmt>();
        while (peek().kind != TokenKind::RBrace) {
            if (peek().kind == TokenKind::End) fail(to_string(peek().loc) + ": unexpected end of file (missing '}')");
            block->stmts.push_back(parse_stmt());
        }
        expect(TokenKind::RBrace, "expected '}'");
        return block;
    }

    // begin ... end
    Token beginTok = expect(TokenKind::KwBegin, "expected 'begin'");
    auto block = std::make_unique<BlockStmt>();
    block->loc = beginTok.loc;
    while (peek().kind != TokenKind::KwEnd) {
        if (peek().kind == TokenKind::End) fail(to_string(peek().loc) + ": unexpected end of file (missing 'end')");
        block->stmts.push_back(parse_stmt());
    }
    expect(TokenKind::KwEnd, "expected 'end'");
    return block;
}

StmtPtr Parser::parse_vardecl() {
    Token typeTok = consume();
    ValueType type = (typeTok.kind == TokenKind::KwInt) ? ValueType::Int : ValueType::Float;

    Token nameTok = expect(TokenKind::Identifier, "expected identifier after type");
    expect(TokenKind::Semicolon, "expected ';' after declaration");

    auto v = std::make_unique<VarDeclStmt>();
    v->loc = typeTok.loc;
    v->declaredType = type;
    v->name = nameTok.lexeme;
    return v;
}

StmtPtr Parser::parse_assign_or_expr_stmt() {
    // For this project: only assignment statements start with identifier.
    Token nameTok = expect(TokenKind::Identifier, "expected identifier");
    if (!accept(TokenKind::Assign)) {
        fail(to_string(peek().loc) + ": expected '=' after identifier (only assignment statements are supported here)");
    }
    auto value = parse_expr();
    expect(TokenKind::Semicolon, "expected ';' after assignment");

    auto a = std::make_unique<AssignStmt>();
    a->loc = nameTok.loc;
    a->name = nameTok.lexeme;
    a->value = std::move(value);
    return a;
}

StmtPtr Parser::parse_if() {
    Token ifTok = expect(TokenKind::KwIf, "expected 'if'");
    expect(TokenKind::LParen, "expected '(' after if");
    auto cond = parse_expr();
    expect(TokenKind::RParen, "expected ')' after condition");

    auto thenBranch = parse_stmt();

    StmtPtr elseBranch;
    if (accept(TokenKind::KwElse)) {
        elseBranch = parse_stmt();
    }

    auto s = std::make_unique<IfStmt>();
    s->loc = ifTok.loc;
    s->cond = std::move(cond);
    s->thenBranch = std::move(thenBranch);
    s->elseBranch = std::move(elseBranch);
    return s;
}

StmtPtr Parser::parse_while() {
    Token wTok = expect(TokenKind::KwWhile, "expected 'while'");
    expect(TokenKind::LParen, "expected '(' after while");
    auto cond = parse_expr();
    expect(TokenKind::RParen, "expected ')' after condition");

    auto body = parse_stmt();

    auto s = std::make_unique<WhileStmt>();
    s->loc = wTok.loc;
    s->cond = std::move(cond);
    s->body = std::move(body);
    return s;
}

StmtPtr Parser::parse_return() {
    Token rTok = expect(TokenKind::KwReturn, "expected 'return'");

    ExprPtr value;
    if (peek().kind != TokenKind::Semicolon) {
        value = parse_expr();
    }
    expect(TokenKind::Semicolon, "expected ';' after return");

    auto s = std::make_unique<ReturnStmt>();
    s->loc = rTok.loc;
    s->value = std::move(value);
    return s;
}

int Parser::precedence(TokenKind op) const {
    switch (op) {
    case TokenKind::Eq:
    case TokenKind::Neq:
        return 10;
    case TokenKind::Lt:
    case TokenKind::Lte:
    case TokenKind::Gt:
    case TokenKind::Gte:
        return 20;
    case TokenKind::Plus:
    case TokenKind::Minus:
        return 30;
    case TokenKind::Star:
    case TokenKind::Slash:
        return 40;
    case TokenKind::Caret:
        return 50;
    default:
        return -1;
    }
}

bool Parser::is_right_associative(TokenKind op) const {
    return op == TokenKind::Caret;
}

bool Parser::is_binary_op(TokenKind op) const {
    return precedence(op) >= 0;
}

ExprPtr Parser::parse_expr(int minPrec) {
    auto lhs = parse_unary();

    while (is_binary_op(peek().kind)) {
        TokenKind op = peek().kind;
        int prec = precedence(op);
        if (prec < minPrec) break;
        consume();

        int nextMinPrec = prec + (is_right_associative(op) ? 0 : 1);
        auto rhs = parse_expr(nextMinPrec);

        auto b = std::make_unique<BinaryExpr>();
        b->loc = lhs->loc;
        b->op = op;
        b->lhs = std::move(lhs);
        b->rhs = std::move(rhs);
        lhs = std::move(b);
    }

    return lhs;
}

ExprPtr Parser::parse_unary() {
    if (peek().kind == TokenKind::Plus || peek().kind == TokenKind::Minus) {
        Token op = consume();
        auto rhs = parse_unary();
        auto u = std::make_unique<UnaryExpr>();
        u->loc = op.loc;
        u->op = op.kind;
        u->rhs = std::move(rhs);
        return u;
    }
    return parse_primary();
}

ExprPtr Parser::parse_primary() {
    Token t = consume();

    if (t.kind == TokenKind::IntLiteral || t.kind == TokenKind::FloatLiteral) {
        auto n = std::make_unique<NumberExpr>();
        n->loc = t.loc;
        n->value = t.numberValue;
        n->isInt = (t.kind == TokenKind::IntLiteral);
        return n;
    }

    if (t.kind == TokenKind::Identifier) {
        // call?
        if (accept(TokenKind::LParen)) {
            auto c = std::make_unique<CallExpr>();
            c->loc = t.loc;
            c->callee = t.lexeme;

            if (peek().kind != TokenKind::RParen) {
                c->args.push_back(parse_expr());
                while (accept(TokenKind::Comma)) {
                    c->args.push_back(parse_expr());
                }
            }
            expect(TokenKind::RParen, "expected ')' after call arguments");
            return c;
        }

        auto id = std::make_unique<IdentExpr>();
        id->loc = t.loc;
        id->name = t.lexeme;
        return id;
    }

    if (t.kind == TokenKind::LParen) {
        auto e = parse_expr();
        expect(TokenKind::RParen, "expected ')' after expression");
        return e;
    }

    fail(to_string(t.loc) + ": unexpected token in expression: " + std::string(token_kind_name(t.kind)));
}

} // namespace cc
