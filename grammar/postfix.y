%{
#include <stdio.h>
#include "calc_tokens.h"

int yylex(void);
void yyerror(const char* s);
%}

%%

input:
    expr { printf("= %g\n", $1); }
;

expr:
    expr expr PLUS   { $$ = $1 + $2; }
  | expr expr MINUS  { $$ = $1 - $2; }
  | expr expr STAR   { $$ = $1 * $2; }
  | NUMBER           { $$ = (double)$1; }
;

%%

void yyerror(const char* s) {
    fprintf(stderr, "parse error: %s\n", s);
}

int main(void) {
    return yyparse();
}
