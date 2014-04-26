%option yylineno
%{
#include <string>
#include "node.h"
#include "parser.hpp"
#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)
extern "C" int yywrap() { }
%}

%%

[ \t\n]                 ;
"int"                   SAVE_TOKEN; return T_TYPE;
"double"                SAVE_TOKEN; return T_TYPE; 
"bool"                  SAVE_TOKEN; return T_TYPE;
"true"                  SAVE_TOKEN; return T_VAL_BOOL;
"false"                 SAVE_TOKEN; return T_VAL_BOOL;
[a-zA-Z_][a-zA-Z0-9_]*  SAVE_TOKEN; return T_IDENTIFIER;
[0-9]+\.[0-9]*          SAVE_TOKEN; return T_VAL_DOUBLE;
[0-9]+                  SAVE_TOKEN; return T_VAL_INTEGER;
"="                     return TOKEN(TEQUAL);
"=="                    return TOKEN(TCEQ);
"!="                    return TOKEN(TCNE);
"<"                     return TOKEN(TCLT);
"<="                    return TOKEN(TCLE);
">"                     return TOKEN(TCGT);
">="                    return TOKEN(TCGE);
"("                     return TOKEN(TLPAREN);
")"                     return TOKEN(TRPAREN);
"{"                     return TOKEN(TLBRACE);
"}"                     return TOKEN(TRBRACE);
"."                     return TOKEN(TDOT);
","                     return TOKEN(TCOMMA);
"+"                     return TOKEN(TPLUS);
"-"                     return TOKEN(TMINUS);
"*"                     return TOKEN(TMUL);
"/"                     return TOKEN(TDIV);
";"                     return TOKEN(TSC);
.                       printf("Unknown token!\n"); yyterminate();

%%
