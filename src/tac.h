#pragma once

#include "ast.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace cc {

struct Operand {
    bool isConst = false;
    double constValue = 0.0;
    std::string name; // variable or temp name (if !isConst)

    static Operand constant(double v) {
        Operand o;
        o.isConst = true;
        o.constValue = v;
        return o;
    }

    static Operand var(std::string n) {
        Operand o;
        o.isConst = false;
        o.name = std::move(n);
        return o;
    }

    std::string to_string() const;
};

enum class TacOp {
    Add, Sub, Mul, Div, Pow,
    Eq, Neq, Lt, Lte, Gt, Gte,
};

std::string to_string(TacOp op);

enum class InstrKind {
    Label,
    Assign,
    Unary,
    Binary,
    IfTrueGoto,
    Goto,
    Call1,
    Return,
};

struct Instr {
    InstrKind kind = InstrKind::Assign;

    // Common fields (only some are used depending on kind)
    std::string label;
    std::string dst;
    Operand a;
    Operand b;
    TacOp op{};

    // For Call1
    std::string callee;

    std::string to_string() const;
};

struct TacProgram {
    std::vector<Instr> code;
};

class TacGenerator {
public:
    TacProgram generate(const Program& program);

private:
    void gen_stmt(const Stmt& stmt);
    Operand gen_expr(const Expr& expr);

    std::string new_temp();
    std::string new_label(const std::string& prefix);

    TacProgram out_;
    int tempCounter_ = 0;
    int labelCounter_ = 0;
};

} // namespace cc
