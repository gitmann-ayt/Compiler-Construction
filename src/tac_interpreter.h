#pragma once

#include "tac.h"

#include <unordered_map>

namespace cc {

class TacInterpreter {
public:
    double run(const TacProgram& p);

private:
    double eval(const Operand& o);

    std::unordered_map<std::string, double> vars_;
};

} // namespace cc
