#pragma once

typedef union {
  int ival;
  double fval;
} YYSTYPE;

extern YYSTYPE yylval;

enum {
  NUMBER = 256,
  PLUS,
  MINUS,
  STAR,
  SLASH,
  CARET,
  LPAREN,
  RPAREN,
  BAD
};
