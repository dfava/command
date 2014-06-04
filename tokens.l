/* Copyright (C) 2014, Daniel S. Fava
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
// copies of the Software, and to permit persons to whom the Software is 
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in 
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
*/
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
"//".*\n                ;
"int"                   SAVE_TOKEN; return T_TYPE;
"double"                SAVE_TOKEN; return T_TYPE; 
"bool"                  SAVE_TOKEN; return T_TYPE;
"true"                  SAVE_TOKEN; return T_VAL_BOOL;
"false"                 SAVE_TOKEN; return T_VAL_BOOL;
"high"                  SAVE_TOKEN; return T_SEC;
"skip"                  return TOKEN(TSKIP);
"if"                    return TOKEN(TIF);
"while"                 return TOKEN(TWHILE);
"else"                  return TOKEN(TELSE);
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
