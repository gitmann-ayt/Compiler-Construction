%option noyywrap nodefault

%{
#include <stdlib.h>
#include "calc_tokens.h"
%}

%%

[0-9]+                 { yylval.ival = atoi(yytext); return NUMBER; }
"+"                   { return PLUS; }
"-"                   { return MINUS; }
"*"                   { return STAR; }
"/"                   { return SLASH; }
"^"                   { return CARET; }
"("                   { return LPAREN; }
")"                   { return RPAREN; }
[ \t\r\n]+            { /* skip */ }
.                      { return BAD; }

%%
