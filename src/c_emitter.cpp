#include "c_emitter.h"

#include <sstream>

namespace cc {

std::string CEmitter::ind(int n) {
    return std::string(static_cast<std::size_t>(n) * 2, ' ');
}

std::string CEmitter::emit(const Program& program) {
    std::ostringstream out;
    out << "#include <math.h>\n";
    out << "\n";
    out << "int main(void) {\n";
    for (auto const& st : program.stmts) {
        out << emit_stmt(*st, 1);
    }
    out << ind(1) << "return 0;\n";
    out << "}\n";
    return out.str();
}

std::string CEmitter::emit_stmt(const Stmt& stmt, int indent) {
    std::ostringstream out;

    if (auto* b = dynamic_cast<const BlockStmt*>(&stmt)) {
        out << ind(indent) << "{\n";
        for (auto const& st : b->stmts) out << emit_stmt(*st, indent + 1);
        out << ind(indent) << "}\n";
        return out.str();
    }

    if (auto* v = dynamic_cast<const VarDeclStmt*>(&stmt)) {
        out << ind(indent) << (v->declaredType == ValueType::Int ? "int" : "double") << " " << v->name << ";\n";
        return out.str();
    }

    if (auto* a = dynamic_cast<const AssignStmt*>(&stmt)) {
        out << ind(indent) << a->name << " = " << emit_expr(*a->value) << ";\n";
        return out.str();
    }

    if (auto* i = dynamic_cast<const IfStmt*>(&stmt)) {
        out << ind(indent) << "if (" << emit_expr(*i->cond) << ")\n";
        out << emit_stmt(*i->thenBranch, indent);
        if (i->elseBranch) {
            out << ind(indent) << "else\n";
            out << emit_stmt(*i->elseBranch, indent);
        }
        return out.str();
    }

    if (auto* w = dynamic_cast<const WhileStmt*>(&stmt)) {
        out << ind(indent) << "while (" << emit_expr(*w->cond) << ")\n";
        out << emit_stmt(*w->body, indent);
        return out.str();
    }

    if (auto* r = dynamic_cast<const ReturnStmt*>(&stmt)) {
        if (r->value) out << ind(indent) << "return " << emit_expr(*r->value) << ";\n";
        else out << ind(indent) << "return 0;\n";
        return out.str();
    }

    // Unknown stmt: emit nothing
    return out.str();
}

std::string CEmitter::emit_expr(const Expr& expr) {
    if (auto* n = dynamic_cast<const NumberExpr*>(&expr)) {
        std::ostringstream out;
        out << n->value;
        return out.str();
    }

    if (auto* id = dynamic_cast<const IdentExpr*>(&expr)) {
        return id->name;
    }

    if (auto* u = dynamic_cast<const UnaryExpr*>(&expr)) {
        std::string op = (u->op == TokenKind::Minus) ? "-" : "+";
        return "(" + op + emit_expr(*u->rhs) + ")";
    }

    if (auto* b = dynamic_cast<const BinaryExpr*>(&expr)) {
        std::string op;
        switch (b->op) {
        case TokenKind::Plus: op = "+"; break;
        case TokenKind::Minus: op = "-"; break;
        case TokenKind::Star: op = "*"; break;
        case TokenKind::Slash: op = "/"; break;
        case TokenKind::Eq: op = "=="; break;
        case TokenKind::Neq: op = "!="; break;
        case TokenKind::Lt: op = "<"; break;
        case TokenKind::Lte: op = "<="; break;
        case TokenKind::Gt: op = ">"; break;
        case TokenKind::Gte: op = ">="; break;
        case TokenKind::Caret:
            // C doesn't have '^' power (that's bitwise XOR). Use pow.
            return "pow(" + emit_expr(*b->lhs) + ", " + emit_expr(*b->rhs) + ")";
        default:
            op = "?";
        }
        return "(" + emit_expr(*b->lhs) + " " + op + " " + emit_expr(*b->rhs) + ")";
    }

    if (auto* c = dynamic_cast<const CallExpr*>(&expr)) {
        if (c->args.empty()) return c->callee + "()";
        return c->callee + "(" + emit_expr(*c->args[0]) + ")";
    }

    return "0";
}

} // namespace cc
