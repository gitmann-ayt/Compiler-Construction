#pragma once

#include "token.h"

#include <string>

namespace cc {

class Lexer {
public:
    explicit Lexer(std::string input);

    const Token& peek();
    Token next();

private:
    Token lex_token();
    void skip_ws_and_comments();

    bool eof() const;
    char cur() const;
    char cur_or(char fallback) const;
    char advance();

    bool match(char ch);

    std::string input_;
    std::size_t pos_ = 0;
    SourceLocation loc_{1, 1};

    bool hasPeek_ = false;
    Token peekTok_{};
};

} // namespace cc
