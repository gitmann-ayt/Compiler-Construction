%{
#include <stdio.h>
#include <math.h>
#include "calc_tokens.h"

int yylex(void);
void yyerror(const char* s);
%}

%left PLUS MINUS
%left STAR SLASH
%right CARET

%%

input:
    expr { printf("= %g\n", $1); }
;

expr:
    expr PLUS expr   { $$ = $1 + $3; }
  | expr MINUS expr  { $$ = $1 - $3; }
  | expr STAR expr   { $$ = $1 * $3; }
  | expr SLASH expr  { $$ = $1 / $3; }
  | expr CARET expr  { $$ = pow($1, $3); }
  | LPAREN expr RPAREN { $$ = $2; }
  | NUMBER { $$ = (double)$1; }
;

%%

void yyerror(const char* s) {
    fprintf(stderr, "parse error: %s\n", s);
}

int main(void) {
    return yyparse();
}
