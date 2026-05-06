#include "tac_interpreter.h"

#include "util.h"

#include <cmath>
#include <unordered_map>

namespace cc {

double TacInterpreter::eval(const Operand& o) {
    if (o.isConst) return o.constValue;
    auto it = vars_.find(o.name);
    if (it == vars_.end()) return 0.0;
    return it->second;
}

double TacInterpreter::run(const TacProgram& p) {
    vars_.clear();

    std::unordered_map<std::string, std::size_t> labels;
    for (std::size_t i = 0; i < p.code.size(); ++i) {
        if (p.code[i].kind == InstrKind::Label) labels[p.code[i].label] = i;
    }

    auto jump_to = [&](const std::string& lab) -> std::size_t {
        auto it = labels.find(lab);
        if (it == labels.end()) fail("interpreter: unknown label '" + lab + "'");
        return it->second;
    };

    for (std::size_t ip = 0; ip < p.code.size(); ++ip) {
        const Instr& ins = p.code[ip];
        switch (ins.kind) {
        case InstrKind::Label:
            break;
        case InstrKind::Assign:
            vars_[ins.dst] = eval(ins.a);
            break;
        case InstrKind::Binary: {
            double a = eval(ins.a);
            double b = eval(ins.b);
            double r = 0.0;
            switch (ins.op) {
            case TacOp::Add: r = a + b; break;
            case TacOp::Sub: r = a - b; break;
            case TacOp::Mul: r = a * b; break;
            case TacOp::Div: r = a / b; break;
            case TacOp::Pow: r = std::pow(a, b); break;
            case TacOp::Eq: r = (a == b) ? 1.0 : 0.0; break;
            case TacOp::Neq: r = (a != b) ? 1.0 : 0.0; break;
            case TacOp::Lt: r = (a < b) ? 1.0 : 0.0; break;
            case TacOp::Lte: r = (a <= b) ? 1.0 : 0.0; break;
            case TacOp::Gt: r = (a > b) ? 1.0 : 0.0; break;
            case TacOp::Gte: r = (a >= b) ? 1.0 : 0.0; break;
            }
            vars_[ins.dst] = r;
            break;
        }
        case InstrKind::Unary:
            // Not used by generator currently
            vars_[ins.dst] = eval(ins.a);
            break;
        case InstrKind::IfTrueGoto:
            if (eval(ins.a) != 0.0) {
                ip = jump_to(ins.label);
            }
            break;
        case InstrKind::Goto:
            ip = jump_to(ins.label);
            break;
        case InstrKind::Call1: {
            double x = eval(ins.a);
            double r = 0.0;
            if (ins.callee == "log") r = std::log(x);
            else if (ins.callee == "exp") r = std::exp(x);
            else fail("interpreter: unknown callee '" + ins.callee + "'");
            vars_[ins.dst] = r;
            break;
        }
        case InstrKind::Return:
            return (ins.a.isConst || !ins.a.name.empty()) ? eval(ins.a) : 0.0;
        }
    }

    return 0.0;
}

} // namespace cc
