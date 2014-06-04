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
%{
    #include "node.h"
    NBlock *programBlock; /* the top level root node of our final AST */

    extern int yylex();
    extern int yylineno;
    void yyerror(const char *s, ...) {
      va_list ap;
      va_start(ap, s);
      fprintf(stderr, "ERR: line %d\n", yylineno);
      vfprintf(stderr, s, ap);
      fprintf(stderr, "\n");
    }
%}

/* Represents the many different ways we can access our data */
%union {
    Node *node;
    NBlock *block;
    NExpression *expr;
    NStatement *stmt;
    NIdentifier *ident;
    NType *type;
    NSecurity *sec;
    NVariableDeclaration *var_decl;
    std::vector<NVariableDeclaration*> *varvec;
    std::vector<NExpression*> *exprvec;
    std::string *string;
    int token;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */
%token <string> T_TYPE T_SEC T_IDENTIFIER T_VAL_INTEGER T_VAL_DOUBLE T_VAL_BOOL
%token <token> TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TCOMMA TDOT
%token <token> TPLUS TMINUS TMUL TDIV TSC
%token <token> TIF TTHEN TELSE TSKIP TWHILE

/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */
%type <type> type
%type <sec> sec
%type <ident> ident
%type <expr> numeric boolean expr 
%type <block> program stmts block
%type <stmt> stmt var_decl

/* Operator precedence */
%nonassoc TTHEN
%nonassoc TELSE
%nonassoc TCEQ TCNE TCLT TCLE TCGT TCGE 
%right TEQUAL
%left TPLUS TMINUS
%left TMUL TDIV

%start program

%%

program : stmts { programBlock = $1; }
        ;
        
stmts : stmt { $$ = new NBlock(); $$->statements.push_back($<stmt>1); }
      | stmts stmt { $1->statements.push_back($<stmt>2); }
      ;

stmt : var_decl
     | expr { $$ = new NExpressionStatement(*$1); }
     ;

block : /*blank*/ { $$ = new NBlock(); }
      | block var_decl { $$->statements.push_back($<stmt>2); }
      | block expr { $$->statements.push_back($<stmt>2); }
      ;

var_decl : type ident TSC { $$ = new NVariableDeclaration(*$1, *$2, *(new NSecurity(""))); $$->lineno = yylineno; }
         | type ident TEQUAL expr TSC{ $$ = new NVariableDeclaration(*$1, *$2, $4, *(new NSecurity(""))); $$->lineno = yylineno; }
         | sec type ident TSC { $$ = new NVariableDeclaration(*$2, *$3, *$1); $$->lineno = yylineno; }
         | sec type ident TEQUAL expr TSC{ $$ = new NVariableDeclaration(*$2, *$3, $5, *$1); $$->lineno = yylineno; }
         ;

type : T_TYPE { $$ = new NType(*$1); delete $1; $$->lineno = yylineno; }
     ;

sec : T_SEC { $$ = new NSecurity(*$1); delete $1; $$->lineno = yylineno; }
    ;
 
expr : ident TEQUAL expr TSC { $$ = new NAssignment(*$<ident>1, *$3); $$->lineno = yylineno; }
     | TSKIP TSC { $$ = new NSkip(); $$->lineno = yylineno; }
     | ident { $<ident>$ = $1; $$->lineno = yylineno; }
     | TIF expr TLBRACE block TRBRACE TELSE TLBRACE block TRBRACE { $$ = new NIfExpression(*$2, *$4, *$8); }
     | TWHILE expr TLBRACE block TRBRACE { $$ = new NWhileExpression(*$2, *$4); }
     | numeric
     | boolean 
     | expr TPLUS expr { $$ = new NBinaryOperator(*$1, $2, *$3); $$->lineno = yylineno; }
     | expr TMINUS expr { $$ = new NBinaryOperator(*$1, $2, *$3); $$->lineno = yylineno; }
     | expr TMUL expr { $$ = new NBinaryOperator(*$1, $2, *$3); $$->lineno = yylineno; }
     | expr TDIV expr { $$ = new NBinaryOperator(*$1, $2, *$3); $$->lineno = yylineno; }
     | expr TCEQ expr { $$ = new NBinaryOperator(*$1, $2, *$3); $$->lineno = yylineno; }
     | expr TCNE expr { $$ = new NBinaryOperator(*$1, $2, *$3); $$->lineno = yylineno; }
     | expr TCLT expr { $$ = new NBinaryOperator(*$1, $2, *$3); $$->lineno = yylineno; }
     | expr TCLE expr { $$ = new NBinaryOperator(*$1, $2, *$3); $$->lineno = yylineno; }
     | expr TCGT expr { $$ = new NBinaryOperator(*$1, $2, *$3); $$->lineno = yylineno; }
     | expr TCGE expr { $$ = new NBinaryOperator(*$1, $2, *$3); $$->lineno = yylineno; }
     | TLPAREN expr TRPAREN { $$ = $2; }
     ;

ident : T_IDENTIFIER { $$ = new NIdentifier(*$1); delete $1; $$->lineno = yylineno; }
      ;

numeric : T_VAL_INTEGER { $$ = new NInteger(atol($1->c_str())); delete $1; $$->lineno = yylineno; }
        | T_VAL_DOUBLE { $$ = new NDouble(atof($1->c_str())); delete $1; $$->lineno = yylineno; }
        ;

boolean : T_VAL_BOOL { $$ = new NBool($1->c_str()); delete $1; $$->lineno = yylineno; }
        ;
%%
