#pragma once

#include "ast.h"

#include <string>

namespace cc {

class CEmitter {
public:
    std::string emit(const Program& program);

private:
    std::string emit_stmt(const Stmt& stmt, int indent);
    std::string emit_expr(const Expr& expr);

    static std::string ind(int n);
};

} // namespace cc
