#include "semantics.h"

#include "util.h"

#include <cmath>
#include <utility>

namespace cc {

void SymbolTable::push_scope() {
    scopes_.push_back({});
}

void SymbolTable::pop_scope() {
    if (!scopes_.empty()) scopes_.pop_back();
}

bool SymbolTable::declare(const std::string& name, ValueType type) {
    if (scopes_.empty()) push_scope();
    auto& scope = scopes_.back();
    if (scope.find(name) != scope.end()) return false;
    scope[name] = Symbol{type};
    return true;
}

const Symbol* SymbolTable::lookup(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return &found->second;
    }
    return nullptr;
}

void SemanticAnalyzer::analyze(Program& program) {
    sym_.push_scope();
    for (auto& st : program.stmts) analyze_stmt(*st);
    sym_.pop_scope();
}

static void require(bool cond, const SourceLocation& loc, const std::string& msg) {
    if (!cond) fail(to_string(loc) + ": " + msg);
}

static bool can_assign(ValueType dst, ValueType src) {
    if (dst == src) return true;
    // implicit promotion int -> float
    if (dst == ValueType::Float && src == ValueType::Int) return true;
    return false;
}

void SemanticAnalyzer::analyze_stmt(Stmt& stmt) {
    if (auto* b = dynamic_cast<BlockStmt*>(&stmt)) {
        sym_.push_scope();
        for (auto& st : b->stmts) analyze_stmt(*st);
        sym_.pop_scope();
        return;
    }

    if (auto* v = dynamic_cast<VarDeclStmt*>(&stmt)) {
        require(v->declaredType == ValueType::Int || v->declaredType == ValueType::Float, v->loc, "only int/float declarations are supported");
        bool ok = sym_.declare(v->name, v->declaredType);
        require(ok, v->loc, "redeclared variable '" + v->name + "' in the same scope");
        return;
    }

    if (auto* a = dynamic_cast<AssignStmt*>(&stmt)) {
        const Symbol* sym = sym_.lookup(a->name);
        require(sym != nullptr, a->loc, "use of undeclared variable '" + a->name + "'");
        ValueType rhs = analyze_expr(*a->value);
        require(can_assign(sym->type, rhs), a->loc, "type mismatch: cannot assign " + to_string(rhs) + " to " + to_string(sym->type));
        return;
    }

    if (auto* i = dynamic_cast<IfStmt*>(&stmt)) {
        ValueType ct = analyze_expr(*i->cond);
        require(ct == ValueType::Bool || is_numeric(ct), i->cond->loc, "if condition must be bool or numeric");
        analyze_stmt(*i->thenBranch);
        if (i->elseBranch) analyze_stmt(*i->elseBranch);
        return;
    }

    if (auto* w = dynamic_cast<WhileStmt*>(&stmt)) {
        ValueType ct = analyze_expr(*w->cond);
        require(ct == ValueType::Bool || is_numeric(ct), w->cond->loc, "while condition must be bool or numeric");
        analyze_stmt(*w->body);
        return;
    }

    if (auto* r = dynamic_cast<ReturnStmt*>(&stmt)) {
        if (r->value) (void)analyze_expr(*r->value);
        return;
    }

    fail(to_string(stmt.loc) + ": unsupported statement kind in semantic analyzer");
}

ValueType SemanticAnalyzer::analyze_expr(Expr& expr) {
    if (auto* n = dynamic_cast<NumberExpr*>(&expr)) {
        expr.type = n->isInt ? ValueType::Int : ValueType::Float;
        return expr.type;
    }

    if (auto* id = dynamic_cast<IdentExpr*>(&expr)) {
        const Symbol* sym = sym_.lookup(id->name);
        require(sym != nullptr, expr.loc, "use of undeclared variable '" + id->name + "'");
        expr.type = sym->type;
        return expr.type;
    }

    if (auto* u = dynamic_cast<UnaryExpr*>(&expr)) {
        ValueType rt = analyze_expr(*u->rhs);
        require(is_numeric(rt), expr.loc, "unary operator requires numeric operand");
        expr.type = rt;
        return expr.type;
    }

    if (auto* b = dynamic_cast<BinaryExpr*>(&expr)) {
        ValueType lt = analyze_expr(*b->lhs);
        ValueType rt = analyze_expr(*b->rhs);

        switch (b->op) {
        case TokenKind::Plus:
        case TokenKind::Minus:
        case TokenKind::Star:
        case TokenKind::Slash:
        case TokenKind::Caret:
            require(is_numeric(lt) && is_numeric(rt), expr.loc, "arithmetic operator requires numeric operands");
            expr.type = (lt == ValueType::Float || rt == ValueType::Float) ? ValueType::Float : ValueType::Int;
            return expr.type;

        case TokenKind::Eq:
        case TokenKind::Neq:
        case TokenKind::Lt:
        case TokenKind::Lte:
        case TokenKind::Gt:
        case TokenKind::Gte:
            require(is_numeric(lt) && is_numeric(rt), expr.loc, "comparison operator requires numeric operands");
            expr.type = ValueType::Bool;
            return expr.type;

        default:
            fail(to_string(expr.loc) + ": unsupported binary operator");
        }
    }

    if (auto* c = dynamic_cast<CallExpr*>(&expr)) {
        require(c->args.size() == 1, expr.loc, "only single-argument calls are supported");
        ValueType at = analyze_expr(*c->args[0]);
        require(is_numeric(at), expr.loc, "function argument must be numeric");

        if (c->callee != "log" && c->callee != "exp") {
            fail(to_string(expr.loc) + ": unknown function '" + c->callee + "' (supported: log, exp)");
        }
        expr.type = ValueType::Float;
        return expr.type;
    }

    fail(to_string(expr.loc) + ": unsupported expression kind in semantic analyzer");
}

} // namespace cc
