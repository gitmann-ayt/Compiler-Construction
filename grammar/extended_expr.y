%{
#include <stdio.h>
#include <math.h>

int yylex(void);
void yyerror(const char* s);

#ifndef YYDEBUG
#define YYDEBUG 1
#endif

int yydebug = 1;
%}

%union {
  double fval;
}

%token <fval> NUM
%token LOG EXP

%left '+' '-'
%left '*' '/'
%right '^'

%type <fval> E T F B

%%

input:
    E { printf("= %g\n", $1); }
;

E:
    E '+' T  { $$ = $1 + $3; }
  | E '-' T  { $$ = $1 - $3; }
  | T        { $$ = $1; }
;

T:
    T '*' F  { $$ = $1 * $3; }
  | T '/' F  { $$ = $1 / $3; }
  | F        { $$ = $1; }
;

F:
    B '^' F  { $$ = pow($1, $3); }
  | B        { $$ = $1; }
;

B:
    '(' E ')'     { $$ = $2; }
  | NUM           { $$ = $1; }
  | LOG '(' E ')' { $$ = log($3); }
  | EXP '(' E ')' { $$ = exp($3); }
;

%%

void yyerror(const char* s) {
    fprintf(stderr, "parse error: %s\n", s);
}

int main(void) {
    return yyparse();
}
