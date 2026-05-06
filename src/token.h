#pragma once

#include "util.h"

#include <string>

namespace cc {

enum class TokenKind {
    End,
    Invalid,

    Identifier,
    IntLiteral,
    FloatLiteral,

    // Keywords
    KwInt,
    KwFloat,
    KwIf,
    KwElse,
    KwWhile,
    KwReturn,

    KwThen,
    KwBegin,
    KwEnd,
    KwProcedure,
    KwFunction,

    // Operators
    Plus,
    Minus,
    Star,
    Slash,
    Caret,

    Assign,      // =
    Eq,          // ==
    Neq,         // !=
    Lt,
    Lte,
    Gt,
    Gte,

    // Delimiters
    LParen,
    RParen,
    LBrace,
    RBrace,
    Semicolon,
    Comma,
};

inline const char* token_kind_name(TokenKind k) {
    switch (k) {
    case TokenKind::End: return "End";
    case TokenKind::Invalid: return "Invalid";
    case TokenKind::Identifier: return "Identifier";
    case TokenKind::IntLiteral: return "IntLiteral";
    case TokenKind::FloatLiteral: return "FloatLiteral";

    case TokenKind::KwInt: return "KwInt";
    case TokenKind::KwFloat: return "KwFloat";
    case TokenKind::KwIf: return "KwIf";
    case TokenKind::KwElse: return "KwElse";
    case TokenKind::KwWhile: return "KwWhile";
    case TokenKind::KwReturn: return "KwReturn";

    case TokenKind::KwThen: return "KwThen";
    case TokenKind::KwBegin: return "KwBegin";
    case TokenKind::KwEnd: return "KwEnd";
    case TokenKind::KwProcedure: return "KwProcedure";
    case TokenKind::KwFunction: return "KwFunction";

    case TokenKind::Plus: return "Plus";
    case TokenKind::Minus: return "Minus";
    case TokenKind::Star: return "Star";
    case TokenKind::Slash: return "Slash";
    case TokenKind::Caret: return "Caret";

    case TokenKind::Assign: return "Assign";
    case TokenKind::Eq: return "Eq";
    case TokenKind::Neq: return "Neq";
    case TokenKind::Lt: return "Lt";
    case TokenKind::Lte: return "Lte";
    case TokenKind::Gt: return "Gt";
    case TokenKind::Gte: return "Gte";

    case TokenKind::LParen: return "LParen";
    case TokenKind::RParen: return "RParen";
    case TokenKind::LBrace: return "LBrace";
    case TokenKind::RBrace: return "RBrace";
    case TokenKind::Semicolon: return "Semicolon";
    case TokenKind::Comma: return "Comma";
    }
    return "<unknown>";
}

struct Token {
    TokenKind kind = TokenKind::Invalid;
    std::string lexeme;
    SourceLocation loc{};

    double numberValue = 0.0;
    bool numberIsInt = false;
};

} // namespace cc
