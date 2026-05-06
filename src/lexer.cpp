#include "lexer.h"

#include <cctype>
#include <cstdlib>
#include <unordered_map>

namespace cc {

static Token make_simple(TokenKind kind, const std::string& lexeme, SourceLocation loc) {
    Token t;
    t.kind = kind;
    t.lexeme = lexeme;
    t.loc = loc;
    return t;
}

Lexer::Lexer(std::string input) : input_(std::move(input)) {}

bool Lexer::eof() const { return pos_ >= input_.size(); }

char Lexer::cur() const { return eof() ? '\0' : input_[pos_]; }

char Lexer::cur_or(char fallback) const { return eof() ? fallback : input_[pos_]; }

char Lexer::advance() {
    if (eof()) return '\0';
    char ch = input_[pos_++];
    if (ch == '\n') {
        loc_.line++;
        loc_.col = 1;
    } else {
        loc_.col++;
    }
    return ch;
}

bool Lexer::match(char ch) {
    if (!eof() && input_[pos_] == ch) {
        advance();
        return true;
    }
    return false;
}

void Lexer::skip_ws_and_comments() {
    while (!eof()) {
        char ch = cur();
        // whitespace
        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
            advance();
            continue;
        }

        // line comment
        if (ch == '/' && (pos_ + 1) < input_.size() && input_[pos_ + 1] == '/') {
            advance();
            advance();
            while (!eof() && cur() != '\n') advance();
            continue;
        }

        // block comment
        if (ch == '/' && (pos_ + 1) < input_.size() && input_[pos_ + 1] == '*') {
            advance();
            advance();
            while (!eof()) {
                if (cur() == '*' && (pos_ + 1) < input_.size() && input_[pos_ + 1] == '/') {
                    advance();
                    advance();
                    break;
                }
                advance();
            }
            continue;
        }

        break;
    }
}

const Token& Lexer::peek() {
    if (!hasPeek_) {
        peekTok_ = lex_token();
        hasPeek_ = true;
    }
    return peekTok_;
}

Token Lexer::next() {
    if (hasPeek_) {
        hasPeek_ = false;
        return peekTok_;
    }
    return lex_token();
}

Token Lexer::lex_token() {
    skip_ws_and_comments();

    SourceLocation startLoc = loc_;

    if (eof()) {
        return make_simple(TokenKind::End, "", startLoc);
    }

    char ch = cur();

    // Identifiers / keywords
    if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
        std::string s;
        while (!eof()) {
            char c = cur();
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
                s.push_back(advance());
            } else {
                break;
            }
        }

        static const std::unordered_map<std::string, TokenKind> keywords = {
            {"int", TokenKind::KwInt},
            {"float", TokenKind::KwFloat},
            {"if", TokenKind::KwIf},
            {"else", TokenKind::KwElse},
            {"while", TokenKind::KwWhile},
            {"return", TokenKind::KwReturn},
            {"then", TokenKind::KwThen},
            {"begin", TokenKind::KwBegin},
            {"end", TokenKind::KwEnd},
            {"procedure", TokenKind::KwProcedure},
            {"function", TokenKind::KwFunction},
        };

        auto it = keywords.find(s);
        if (it != keywords.end()) {
            return make_simple(it->second, s, startLoc);
        }

        Token t = make_simple(TokenKind::Identifier, s, startLoc);
        return t;
    }

    // Numbers
    if (std::isdigit(static_cast<unsigned char>(ch))) {
        std::string s;
        while (!eof() && std::isdigit(static_cast<unsigned char>(cur()))) {
            s.push_back(advance());
        }

        bool isFloat = false;
        if (!eof() && cur() == '.' && (pos_ + 1) < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_ + 1]))) {
            isFloat = true;
            s.push_back(advance()); // '.'
            while (!eof() && std::isdigit(static_cast<unsigned char>(cur()))) {
                s.push_back(advance());
            }
        }

        Token t;
        t.loc = startLoc;
        t.lexeme = s;
        if (isFloat) {
            t.kind = TokenKind::FloatLiteral;
            t.numberValue = std::strtod(s.c_str(), nullptr);
            t.numberIsInt = false;
        } else {
            t.kind = TokenKind::IntLiteral;
            t.numberValue = std::strtod(s.c_str(), nullptr);
            t.numberIsInt = true;
        }
        return t;
    }

    // Operators / delimiters
    switch (ch) {
    case '+':
        advance();
        return make_simple(TokenKind::Plus, "+", startLoc);
    case '-':
        advance();
        return make_simple(TokenKind::Minus, "-", startLoc);
    case '*':
        advance();
        return make_simple(TokenKind::Star, "*", startLoc);
    case '/':
        advance();
        return make_simple(TokenKind::Slash, "/", startLoc);
    case '^':
        advance();
        return make_simple(TokenKind::Caret, "^", startLoc);
    case '(':
        advance();
        return make_simple(TokenKind::LParen, "(", startLoc);
    case ')':
        advance();
        return make_simple(TokenKind::RParen, ")", startLoc);
    case '{':
        advance();
        return make_simple(TokenKind::LBrace, "{", startLoc);
    case '}':
        advance();
        return make_simple(TokenKind::RBrace, "}", startLoc);
    case ';':
        advance();
        return make_simple(TokenKind::Semicolon, ";", startLoc);
    case ',':
        advance();
        return make_simple(TokenKind::Comma, ",", startLoc);
    case '=':
        advance();
        if (match('=')) return make_simple(TokenKind::Eq, "==", startLoc);
        return make_simple(TokenKind::Assign, "=", startLoc);
    case '!':
        advance();
        if (match('=')) return make_simple(TokenKind::Neq, "!=", startLoc);
        return make_simple(TokenKind::Invalid, "!", startLoc);
    case '<':
        advance();
        if (match('=')) return make_simple(TokenKind::Lte, "<=", startLoc);
        return make_simple(TokenKind::Lt, "<", startLoc);
    case '>':
        advance();
        if (match('=')) return make_simple(TokenKind::Gte, ">=", startLoc);
        return make_simple(TokenKind::Gt, ">", startLoc);
    default:
        break;
    }

    // Unrecognized
    std::string bad(1, advance());
    return make_simple(TokenKind::Invalid, bad, startLoc);
}

} // namespace cc
