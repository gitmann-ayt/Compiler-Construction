#include "ast.h"

#include <sstream>

namespace cc {

static void indent(std::ostringstream& out, int n) {
    for (int i = 0; i < n; ++i) out << "  ";
}

static void dump_expr(std::ostringstream& out, const Expr& e, int depth);
static void dump_stmt(std::ostringstream& out, const Stmt& s, int depth);

static std::string op_to_str(TokenKind op) {
    switch (op) {
    case TokenKind::Plus: return "+";
    case TokenKind::Minus: return "-";
    case TokenKind::Star: return "*";
    case TokenKind::Slash: return "/";
    case TokenKind::Caret: return "^";
    case TokenKind::Eq: return "==";
    case TokenKind::Neq: return "!=";
    case TokenKind::Lt: return "<";
    case TokenKind::Lte: return "<=";
    case TokenKind::Gt: return ">";
    case TokenKind::Gte: return ">=";
    default: return token_kind_name(op);
    }
}

static void dump_expr(std::ostringstream& out, const Expr& e, int depth) {
    if (auto* n = dynamic_cast<const NumberExpr*>(&e)) {
        indent(out, depth);
        out << "Number(" << n->value << (n->isInt ? ", int" : ", float") << ") : " << to_string(e.type) << "\n";
        return;
    }
    if (auto* id = dynamic_cast<const IdentExpr*>(&e)) {
        indent(out, depth);
        out << "Ident(" << id->name << ") : " << to_string(e.type) << "\n";
        return;
    }
    if (auto* u = dynamic_cast<const UnaryExpr*>(&e)) {
        indent(out, depth);
        out << "Unary(" << op_to_str(u->op) << ") : " << to_string(e.type) << "\n";
        dump_expr(out, *u->rhs, depth + 1);
        return;
    }
    if (auto* b = dynamic_cast<const BinaryExpr*>(&e)) {
        indent(out, depth);
        out << "Binary(" << op_to_str(b->op) << ") : " << to_string(e.type) << "\n";
        dump_expr(out, *b->lhs, depth + 1);
        dump_expr(out, *b->rhs, depth + 1);
        return;
    }
    if (auto* c = dynamic_cast<const CallExpr*>(&e)) {
        indent(out, depth);
        out << "Call(" << c->callee << ") : " << to_string(e.type) << "\n";
        for (auto const& arg : c->args) dump_expr(out, *arg, depth + 1);
        return;
    }

    indent(out, depth);
    out << "<unknown expr>\n";
}

static void dump_stmt(std::ostringstream& out, const Stmt& s, int depth) {
    if (auto* b = dynamic_cast<const BlockStmt*>(&s)) {
        indent(out, depth);
        out << "Block\n";
        for (auto const& st : b->stmts) dump_stmt(out, *st, depth + 1);
        return;
    }
    if (auto* v = dynamic_cast<const VarDeclStmt*>(&s)) {
        indent(out, depth);
        out << "VarDecl(" << to_string(v->declaredType) << " " << v->name << ")\n";
        return;
    }
    if (auto* a = dynamic_cast<const AssignStmt*>(&s)) {
        indent(out, depth);
        out << "Assign(" << a->name << ")\n";
        dump_expr(out, *a->value, depth + 1);
        return;
    }
    if (auto* i = dynamic_cast<const IfStmt*>(&s)) {
        indent(out, depth);
        out << "If\n";
        indent(out, depth + 1);
        out << "Cond\n";
        dump_expr(out, *i->cond, depth + 2);
        indent(out, depth + 1);
        out << "Then\n";
        dump_stmt(out, *i->thenBranch, depth + 2);
        if (i->elseBranch) {
            indent(out, depth + 1);
            out << "Else\n";
            dump_stmt(out, *i->elseBranch, depth + 2);
        }
        return;
    }
    if (auto* w = dynamic_cast<const WhileStmt*>(&s)) {
        indent(out, depth);
        out << "While\n";
        indent(out, depth + 1);
        out << "Cond\n";
        dump_expr(out, *w->cond, depth + 2);
        indent(out, depth + 1);
        out << "Body\n";
        dump_stmt(out, *w->body, depth + 2);
        return;
    }
    if (auto* r = dynamic_cast<const ReturnStmt*>(&s)) {
        indent(out, depth);
        out << "Return\n";
        if (r->value) dump_expr(out, *r->value, depth + 1);
        return;
    }

    indent(out, depth);
    out << "<unknown stmt>\n";
}

std::string dump_ast(const Program& program) {
    std::ostringstream out;
    out << "Program\n";
    for (auto const& st : program.stmts) dump_stmt(out, *st, 1);
    return out.str();
}

} // namespace cc
