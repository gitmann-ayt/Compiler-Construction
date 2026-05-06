#include "optimizer.h"

#include "util.h"

#include <cmath>
#include <optional>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace cc {

static bool is_pure(const Instr& i) {
    // In this project, all ops are pure (no IO). Keep it simple.
    return i.kind == InstrKind::Assign || i.kind == InstrKind::Binary || i.kind == InstrKind::Unary || i.kind == InstrKind::Call1;
}

static bool defines_dst(const Instr& i) {
    return i.kind == InstrKind::Assign || i.kind == InstrKind::Binary || i.kind == InstrKind::Unary || i.kind == InstrKind::Call1;
}

static bool uses_operand(const Operand& o) {
    return !o.isConst && !o.name.empty();
}

static std::unordered_map<std::string, std::size_t> label_index(const TacProgram& p) {
    std::unordered_map<std::string, std::size_t> m;
    for (std::size_t i = 0; i < p.code.size(); ++i) {
        if (p.code[i].kind == InstrKind::Label) {
            m[p.code[i].label] = i;
        }
    }
    return m;
}

static std::vector<std::vector<std::size_t>> successors(const TacProgram& p, const std::unordered_map<std::string, std::size_t>& labels) {
    std::vector<std::vector<std::size_t>> succ(p.code.size());
    for (std::size_t i = 0; i < p.code.size(); ++i) {
        const Instr& ins = p.code[i];
        auto add_next = [&]() {
            if (i + 1 < p.code.size()) succ[i].push_back(i + 1);
        };

        switch (ins.kind) {
        case InstrKind::Goto: {
            auto it = labels.find(ins.label);
            if (it != labels.end()) succ[i].push_back(it->second);
            break;
        }
        case InstrKind::IfTrueGoto: {
            auto it = labels.find(ins.label);
            if (it != labels.end()) succ[i].push_back(it->second);
            add_next();
            break;
        }
        case InstrKind::Return:
            break;
        default:
            add_next();
            break;
        }
    }
    return succ;
}

static std::unordered_set<std::size_t> reachable_indices(const TacProgram& p) {
    if (p.code.empty()) return {};

    auto labels = label_index(p);
    auto succ = successors(p, labels);

    std::unordered_set<std::size_t> vis;
    std::queue<std::size_t> q;
    q.push(0);
    vis.insert(0);

    while (!q.empty()) {
        std::size_t u = q.front();
        q.pop();
        for (auto v : succ[u]) {
            if (!vis.count(v)) {
                vis.insert(v);
                q.push(v);
            }
        }
    }
    return vis;
}

static std::optional<double> eval_bin(TacOp op, double a, double b) {
    switch (op) {
    case TacOp::Add: return a + b;
    case TacOp::Sub: return a - b;
    case TacOp::Mul: return a * b;
    case TacOp::Div: return a / b;
    case TacOp::Pow: return std::pow(a, b);

    case TacOp::Eq: return (a == b) ? 1.0 : 0.0;
    case TacOp::Neq: return (a != b) ? 1.0 : 0.0;
    case TacOp::Lt: return (a < b) ? 1.0 : 0.0;
    case TacOp::Lte: return (a <= b) ? 1.0 : 0.0;
    case TacOp::Gt: return (a > b) ? 1.0 : 0.0;
    case TacOp::Gte: return (a >= b) ? 1.0 : 0.0;
    }
    return std::nullopt;
}

static std::optional<double> eval_call1(const std::string& callee, double x) {
    if (callee == "log") return std::log(x);
    if (callee == "exp") return std::exp(x);
    return std::nullopt;
}

TacProgram Optimizer::optimize(const TacProgram& in, const OptimizerOptions& opt) {
    TacProgram p = in;

    if (opt.unreachableElimination) eliminate_unreachable(p);
    if (opt.constantFolding) constant_folding(p);
    if (opt.constantPropagation) constant_propagation(p);
    if (opt.deadCodeElimination) dead_code_elimination(p);

    // Clean up again after DCE
    if (opt.unreachableElimination) eliminate_unreachable(p);
    return p;
}

void Optimizer::eliminate_unreachable(TacProgram& p) {
    auto reach = reachable_indices(p);
    if (reach.empty()) return;

    // Ensure labels that are jump targets are kept (even if the label itself seems unreachable due to conservative issues)
    std::unordered_set<std::string> targets;
    for (auto const& ins : p.code) {
        if (ins.kind == InstrKind::Goto || ins.kind == InstrKind::IfTrueGoto) targets.insert(ins.label);
    }

    std::vector<Instr> out;
    out.reserve(p.code.size());
    for (std::size_t i = 0; i < p.code.size(); ++i) {
        const auto& ins = p.code[i];
        if (reach.count(i)) {
            out.push_back(ins);
            continue;
        }
        if (ins.kind == InstrKind::Label && targets.count(ins.label)) {
            out.push_back(ins);
        }
    }
    p.code = std::move(out);
}

void Optimizer::constant_folding(TacProgram& p) {
    for (auto& ins : p.code) {
        if (ins.kind == InstrKind::Binary && ins.a.isConst && ins.b.isConst) {
            auto r = eval_bin(ins.op, ins.a.constValue, ins.b.constValue);
            if (r) {
                ins.kind = InstrKind::Assign;
                ins.a = Operand::constant(*r);
            }
        }
        if (ins.kind == InstrKind::Call1 && ins.a.isConst) {
            auto r = eval_call1(ins.callee, ins.a.constValue);
            if (r) {
                ins.kind = InstrKind::Assign;
                ins.a = Operand::constant(*r);
            }
        }
    }
}

void Optimizer::constant_propagation(TacProgram& p) {
    // Simple forward propagation along linear order; conservative around labels/jumps.
    std::unordered_map<std::string, double> env;

    auto kill_all = [&]() { env.clear(); };

    for (auto& ins : p.code) {
        if (ins.kind == InstrKind::Label) {
            kill_all();
            continue;
        }
        if (ins.kind == InstrKind::Goto || ins.kind == InstrKind::IfTrueGoto || ins.kind == InstrKind::Return) {
            if (uses_operand(ins.a)) {
                auto it = env.find(ins.a.name);
                if (it != env.end()) ins.a = Operand::constant(it->second);
            }
            if (ins.kind != InstrKind::Return) kill_all();
            continue;
        }

        auto rewrite = [&](Operand& o) {
            if (uses_operand(o)) {
                auto it = env.find(o.name);
                if (it != env.end()) o = Operand::constant(it->second);
            }
        };

        rewrite(ins.a);
        rewrite(ins.b);

        if (defines_dst(ins)) {
            env.erase(ins.dst);
            if (ins.kind == InstrKind::Assign && ins.a.isConst) {
                env[ins.dst] = ins.a.constValue;
            }
        }
    }
}

void Optimizer::dead_code_elimination(TacProgram& p) {
    if (p.code.empty()) return;

    auto labels = label_index(p);
    auto succ = successors(p, labels);

    std::vector<std::unordered_set<std::string>> use(p.code.size());
    std::vector<std::unordered_set<std::string>> def(p.code.size());

    for (std::size_t i = 0; i < p.code.size(); ++i) {
        const Instr& ins = p.code[i];
        if (defines_dst(ins) && !ins.dst.empty()) def[i].insert(ins.dst);
        if (uses_operand(ins.a)) use[i].insert(ins.a.name);
        if (uses_operand(ins.b)) use[i].insert(ins.b.name);
    }

    std::vector<std::unordered_set<std::string>> liveIn(p.code.size()), liveOut(p.code.size());

    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t i = p.code.size(); i-- > 0;) {
            std::unordered_set<std::string> outSet;
            for (auto s : succ[i]) {
                outSet.insert(liveIn[s].begin(), liveIn[s].end());
            }

            std::unordered_set<std::string> inSet = use[i];
            for (auto const& v : outSet) {
                if (!def[i].count(v)) inSet.insert(v);
            }

            if (outSet != liveOut[i] || inSet != liveIn[i]) {
                liveOut[i] = std::move(outSet);
                liveIn[i] = std::move(inSet);
                changed = true;
            }
        }
    }

    std::vector<Instr> out;
    out.reserve(p.code.size());

    for (std::size_t i = 0; i < p.code.size(); ++i) {
        const Instr& ins = p.code[i];
        if (defines_dst(ins) && is_pure(ins) && !ins.dst.empty()) {
            if (!liveOut[i].count(ins.dst)) {
                continue; // dead
            }
        }
        out.push_back(ins);
    }

    p.code = std::move(out);
}

} // namespace cc
