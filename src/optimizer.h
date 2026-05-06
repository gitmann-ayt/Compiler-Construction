#pragma once

#include "tac.h"

#include <string>

namespace cc {

struct OptimizerOptions {
    bool constantFolding = true;
    bool constantPropagation = true;
    bool deadCodeElimination = true;
    bool unreachableElimination = true;
};

class Optimizer {
public:
    TacProgram optimize(const TacProgram& in, const OptimizerOptions& opt);

private:
    void eliminate_unreachable(TacProgram& p);
    void constant_folding(TacProgram& p);
    void constant_propagation(TacProgram& p);
    void dead_code_elimination(TacProgram& p);
};

} // namespace cc
