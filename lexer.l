%{
#include "parser.tab.h"
#include "vibe.h"
#include <string.h>
#include <stdlib.h>
%}

%%

"vibe"      { return VIBE; }
"plot"      { return PLOT; }
"spill"     { return SPILL; }
"drop"      { return DROP; }
"yah"       { return YAH; }
"maybe"     { return MAYBE; }
"nah"       { return NAH; }
"runthru"   { return RUNTHRU; }
"onrepeat"  { return ONREPEAT; }
"dostart"   { return DOSTART; }
"doend"     { return DOEND; }
"grab"      { return GRAB; }
"scoop"     { return SCOOP; }
"dip"       { return DIP; }
"slide"     { return SLIDE; }

"void"      { return VOID_TYPE; }
"int"       { return INT_TYPE; }
"float"     { return FLOAT_TYPE; }
"string"    { return STRING_TYPE; }
"bool"      { return BOOL_TYPE; }
"legit"     { yylval.boolval = 1; return BOOLVAL; }
"cap"       { yylval.boolval = 0; return BOOLVAL; }

[0-9]+          { yylval.ival = atoi(yytext); return INT; }
[0-9]+\.[0-9]+  { yylval.fval = atof(yytext); return FLOAT; }
\"[^\"]*\"      { yylval.sval = strdup(yytext+1); 
                  yylval.sval[strlen(yylval.sval)-1] = '\0'; 
                  return STRING; }

[a-zA-Z_][a-zA-Z0-9_]*  { yylval.sval = strdup(yytext); return IDENT; }

"=="    { return EQ; }
"!="    { return NEQ; }
"<="    { return LE; }
">="    { return GE; }
"="     { return ASSIGN; }
"<"     { return LT; }
">"     { return GT; }
"+"     { return PLUS; }
"-"     { return MINUS; }
"*"     { return MUL; }
"/"     { return DIV; }
"&&"    { return AND; }
"||"    { return OR; }
"!"     { return NOT; }

":"     { return COLON; }
","     { return COMMA; }
";"     { return SEMICOLON; }
"("     { return LPAREN; }
")"     { return RPAREN; }
"{"     { return LBRACE; }
"}"     { return RBRACE; }

[ \t\n]+    { /* ignore whitespace */ }
.       { printf("Lex error: %s\n", yytext); }

%%

int yywrap() { return 1; }
