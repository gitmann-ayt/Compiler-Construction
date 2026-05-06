#pragma once

#include <string>

namespace cc {

enum class ValueType {
    Unknown,
    Int,
    Float,
    Bool,
    Void,
};

inline std::string to_string(ValueType t) {
    switch (t) {
    case ValueType::Unknown: return "unknown";
    case ValueType::Int: return "int";
    case ValueType::Float: return "float";
    case ValueType::Bool: return "bool";
    case ValueType::Void: return "void";
    }
    return "unknown";
}

inline bool is_numeric(ValueType t) {
    return t == ValueType::Int || t == ValueType::Float;
}

} // namespace cc
