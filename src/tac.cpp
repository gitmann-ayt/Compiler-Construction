#include "tac.h"

#include "util.h"

#include <cmath>
#include <sstream>

namespace cc {

std::string Operand::to_string() const {
    if (isConst) {
        std::ostringstream oss;
        oss << constValue;
        return oss.str();
    }
    return name;
}

std::string to_string(TacOp op) {
    switch (op) {
    case TacOp::Add: return "+";
    case TacOp::Sub: return "-";
    case TacOp::Mul: return "*";
    case TacOp::Div: return "/";
    case TacOp::Pow: return "^";
    case TacOp::Eq: return "==";
    case TacOp::Neq: return "!=";
    case TacOp::Lt: return "<";
    case TacOp::Lte: return "<=";
    case TacOp::Gt: return ">";
    case TacOp::Gte: return ">=";
    }
    return "?";
}

std::string Instr::to_string() const {
    std::ostringstream out;
    switch (kind) {
    case InstrKind::Label:
        out << label << ":";
        break;
    case InstrKind::Assign:
        out << dst << " = " << a.to_string();
        break;
    case InstrKind::Unary:
        out << dst << " = " << ::cc::to_string(op) << " " << a.to_string();
        break;
    case InstrKind::Binary:
        out << dst << " = " << a.to_string() << " " << ::cc::to_string(op) << " " << b.to_string();
        break;
    case InstrKind::IfTrueGoto:
        out << "if " << a.to_string() << " goto " << label;
        break;
    case InstrKind::Goto:
        out << "goto " << label;
        break;
    case InstrKind::Call1:
        out << dst << " = call " << callee << ", " << a.to_string();
        break;
    case InstrKind::Return:
        out << "return";
        if (a.isConst || !a.name.empty()) out << " " << a.to_string();
        break;
    }
    return out.str();
}

static TacOp map_binop(TokenKind op) {
    switch (op) {
    case TokenKind::Plus: return TacOp::Add;
    case TokenKind::Minus: return TacOp::Sub;
    case TokenKind::Star: return TacOp::Mul;
    case TokenKind::Slash: return TacOp::Div;
    case TokenKind::Caret: return TacOp::Pow;
    case TokenKind::Eq: return TacOp::Eq;
    case TokenKind::Neq: return TacOp::Neq;
    case TokenKind::Lt: return TacOp::Lt;
    case TokenKind::Lte: return TacOp::Lte;
    case TokenKind::Gt: return TacOp::Gt;
    case TokenKind::Gte: return TacOp::Gte;
    default:
        fail("internal: unsupported binary op for TAC");
    }
}

TacProgram TacGenerator::generate(const Program& program) {
    out_.code.clear();
    tempCounter_ = 0;
    labelCounter_ = 0;

    for (auto const& st : program.stmts) gen_stmt(*st);
    return out_;
}

std::string TacGenerator::new_temp() {
    return "t" + std::to_string(++tempCounter_);
}

std::string TacGenerator::new_label(const std::string& prefix) {
    return prefix + std::to_string(++labelCounter_);
}

void TacGenerator::gen_stmt(const Stmt& stmt) {
    if (auto* b = dynamic_cast<const BlockStmt*>(&stmt)) {
        for (auto const& st : b->stmts) gen_stmt(*st);
        return;
    }

    if (dynamic_cast<const VarDeclStmt*>(&stmt)) {
        // No TAC emitted for declaration (symbol table handles it)
        return;
    }

    if (auto* a = dynamic_cast<const AssignStmt*>(&stmt)) {
        Operand rhs = gen_expr(*a->value);
        Instr i;
        i.kind = InstrKind::Assign;
        i.dst = a->name;
        i.a = rhs;
        out_.code.push_back(i);
        return;
    }

    if (auto* r = dynamic_cast<const ReturnStmt*>(&stmt)) {
        Instr i;
        i.kind = InstrKind::Return;
        if (r->value) i.a = gen_expr(*r->value);
        out_.code.push_back(i);
        return;
    }

    if (auto* i = dynamic_cast<const IfStmt*>(&stmt)) {
        Operand cond = gen_expr(*i->cond);
        std::string lThen = new_label("L_then_");
        std::string lElse = new_label("L_else_");
        std::string lEnd = new_label("L_end_");

        Instr ifTrue;
        ifTrue.kind = InstrKind::IfTrueGoto;
        ifTrue.a = cond;
        ifTrue.label = lThen;
        out_.code.push_back(ifTrue);

        Instr goElse;
        goElse.kind = InstrKind::Goto;
        goElse.label = lElse;
        out_.code.push_back(goElse);

        Instr labThen;
        labThen.kind = InstrKind::Label;
        labThen.label = lThen;
        out_.code.push_back(labThen);
        gen_stmt(*i->thenBranch);

        Instr goEnd;
        goEnd.kind = InstrKind::Goto;
        goEnd.label = lEnd;
        out_.code.push_back(goEnd);

        Instr labElse;
        labElse.kind = InstrKind::Label;
        labElse.label = lElse;
        out_.code.push_back(labElse);
        if (i->elseBranch) gen_stmt(*i->elseBranch);

        Instr labEnd;
        labEnd.kind = InstrKind::Label;
        labEnd.label = lEnd;
        out_.code.push_back(labEnd);
        return;
    }

    if (auto* w = dynamic_cast<const WhileStmt*>(&stmt)) {
        std::string lHead = new_label("L_while_head_");
        std::string lBody = new_label("L_while_body_");
        std::string lEnd = new_label("L_while_end_");

        out_.code.push_back(Instr{InstrKind::Label, lHead});

        Operand cond = gen_expr(*w->cond);
        Instr ifTrue;
        ifTrue.kind = InstrKind::IfTrueGoto;
        ifTrue.a = cond;
        ifTrue.label = lBody;
        out_.code.push_back(ifTrue);

        Instr goEnd;
        goEnd.kind = InstrKind::Goto;
        goEnd.label = lEnd;
        out_.code.push_back(goEnd);

        out_.code.push_back(Instr{InstrKind::Label, lBody});
        gen_stmt(*w->body);

        Instr goHead;
        goHead.kind = InstrKind::Goto;
        goHead.label = lHead;
        out_.code.push_back(goHead);

        out_.code.push_back(Instr{InstrKind::Label, lEnd});
        return;
    }

    fail("internal: unsupported statement for TAC generation");
}

Operand TacGenerator::gen_expr(const Expr& expr) {
    if (auto* n = dynamic_cast<const NumberExpr*>(&expr)) {
        return Operand::constant(n->value);
    }

    if (auto* id = dynamic_cast<const IdentExpr*>(&expr)) {
        return Operand::var(id->name);
    }

    if (auto* u = dynamic_cast<const UnaryExpr*>(&expr)) {
        Operand rhs = gen_expr(*u->rhs);
        if (u->op == TokenKind::Plus) return rhs;
        if (u->op == TokenKind::Minus) {
            // Prefer constant fold early if possible
            if (rhs.isConst) return Operand::constant(-rhs.constValue);
            std::string t = new_temp();
            Instr i;
            i.kind = InstrKind::Unary;
            i.dst = t;
            i.op = TacOp::Sub; // represent unary minus as 0 - x in optimizer/interpreter
            i.a = Operand::constant(0.0);
            i.b = rhs;
            i.kind = InstrKind::Binary;
            out_.code.push_back(i);
            return Operand::var(t);
        }
        fail("internal: unsupported unary operator in TAC");
    }

    if (auto* b = dynamic_cast<const BinaryExpr*>(&expr)) {
        Operand lhs = gen_expr(*b->lhs);
        Operand rhs = gen_expr(*b->rhs);
        std::string t = new_temp();

        Instr i;
        i.kind = InstrKind::Binary;
        i.dst = t;
        i.a = lhs;
        i.b = rhs;
        i.op = map_binop(b->op);
        out_.code.push_back(i);
        return Operand::var(t);
    }

    if (auto* c = dynamic_cast<const CallExpr*>(&expr)) {
        Operand arg = gen_expr(*c->args[0]);
        std::string t = new_temp();
        Instr i;
        i.kind = InstrKind::Call1;
        i.dst = t;
        i.callee = c->callee;
        i.a = arg;
        out_.code.push_back(i);
        return Operand::var(t);
    }

    fail("internal: unsupported expression for TAC generation");
}

} // namespace cc
