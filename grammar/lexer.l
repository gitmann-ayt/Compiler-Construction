%option noyywrap nodefault yylineno

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For integration with Bison: include the generated header.
   Example:
     #include "parser.tab.h"
   Then return token codes.
*/

%}

DIGIT      [0-9]
ID_START   [a-zA-Z_]
ID_CONT    [a-zA-Z0-9_]

%%

"int"|"float"|"if"|"else"|"while"|"return"|"then"|"begin"|"end"|"procedure"|"function" {
    printf("KW(%s)\n", yytext);
}

{ID_START}{ID_CONT}*   { printf("ID(%s)\n", yytext); }

{DIGIT}+\.{DIGIT}+    { printf("FLOAT(%s)\n", yytext); }
{DIGIT}+              { printf("INT(%s)\n", yytext); }

"=="|"!="            { printf("OP(%s)\n", yytext); }
"<="|">="            { printf("OP(%s)\n", yytext); }
"+"|"-"|"*"|"/"|"^"|"="|"<"|">" { printf("OP(%s)\n", yytext); }

"("|")"|"{"|"}"|";"|"," { printf("DELIM(%s)\n", yytext); }

"//".*               { /* discard line comment */ }
"/*"([^*]|\*+[^*/])*"*/"  { /* discard block comment */ }

[ \t\r\n]+            { /* discard whitespace */ }

.                     {
    fprintf(stderr, "Lexer error at line %d: unrecognised character '%s'\n", yylineno, yytext);
}

%%

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    yylex();
    return 0;
}
