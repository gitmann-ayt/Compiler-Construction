%{
#include <stdio.h>
#include <math.h>
#include "calc_tokens.h"

int yylex(void);
void yyerror(const char* s);
%}

%%

input:
    expr { printf("= %g\n", $1); }
;

expr:
    PLUS expr expr   { $$ = $2 + $3; }
  | MINUS expr expr  { $$ = $2 - $3; }
  | STAR expr expr   { $$ = $2 * $3; }
  | SLASH expr expr  { $$ = $2 / $3; }
  | CARET expr expr  { $$ = pow($2, $3); }
  | NUMBER { $$ = (double)$1; }
;

%%

void yyerror(const char* s) {
    fprintf(stderr, "parse error: %s\n", s);
}

int main(void) {
    return yyparse();
}
