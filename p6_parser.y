%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <algorithm>
#include <memory>
#include <cstring>
#include <cdk/compiler.h>
#include <cdk/types/types.h>
#include <cdk/types/balanced3_type.h>
#include <cdk/types/takum3_type.h>
#include ".auto/all_nodes.h"
#define LINE                         compiler->scanner()->lineno()
#define yylex()                      compiler->scanner()->scan()
#define yyerror(compiler, s)         compiler->scanner()->error(s)
//-- don't change *any* of these --- END!
%}

%parse-param {std::shared_ptr<cdk::compiler> compiler}

%union {
  //--- don't change *any* of these: if you do, you'll break the compiler.
  YYSTYPE() : type(cdk::primitive_type::create(0, cdk::TYPE_VOID)) {}
  ~YYSTYPE() {}
  YYSTYPE(const YYSTYPE &other) { *this = other; }
  YYSTYPE& operator=(const YYSTYPE &other) { type = other.type; return *this; }

  std::shared_ptr<cdk::basic_type> type;        /* expression type */
  //-- don't change *any* of these --- END!

  int                   i;          /* integer value (legacy; not emitted by P6 scanner) */
  cdk::balanced3_type::value_type *b; /* P6 integer literal (balanced ternary) */
  cdk::takum3_type::value_type    *t; /* P6 real literal (Takum3) */
  std::string          *s;          /* symbol name or string literal */
  cdk::basic_node      *node;       /* node pointer */
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;
};

%token <b> tINTEGER
%token <t> tREAL
%token <s> tIDENTIFIER tSTRING
%token tWHILE tIF tPRINT tREAD tBEGIN tEND
/* Tokens devolvidos pelo scanner P6 -- regras Bison na Etapa 2 */
%token tELIF tSTOP tNEXT tRETURN
%token tINPUT tNULL tSIZEOF
%token tTYPE_INT tTYPE_REAL tTYPE_STRING tTYPE_VOID
%token tEXTERN tFORWARD tPUBLIC tAUTO
%token tAND tOR tARROW tPRINTLN

%nonassoc tIFX
%nonassoc tELSE

%right '='
%left tGE tLE tEQ tNE '>' '<'
%left '+' '-'
%left '*' '/' '%'
%nonassoc tUNARY

%type <node> stmt program
%type <sequence> stmts
%type <expression> expr
%type <lvalue> lval

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

program : tBEGIN stmts tEND { compiler->ast(new p6::program_node(LINE, new p6::block_node(LINE, new cdk::sequence_node(LINE), $2))); }
        ;

stmts : stmt       { $$ = new cdk::sequence_node(LINE, $1); }
      | stmts stmt { $$ = new cdk::sequence_node(LINE, $2, $1); }
      ;

stmt : expr ';'                         { $$ = new p6::evaluation_node(LINE, $1); }
     | tPRINT expr ';'                  { $$ = new p6::write_node(LINE, new cdk::sequence_node(LINE, $2), true); }
     | tREAD lval ';'                   { $$ = new p6::evaluation_node(LINE, new cdk::assignment_node(LINE, $2, new p6::input_node(LINE))); }
     | tWHILE '(' expr ')' stmt         { $$ = new p6::while_node(LINE, $3, $5); }
     | tIF '(' expr ')' stmt %prec tIFX { $$ = new p6::if_node(LINE, $3, $5); }
     | tIF '(' expr ')' stmt tELSE stmt { $$ = new p6::if_else_node(LINE, $3, $5, $7); }
     | '{' stmts '}'                    { $$ = $2; }
     ;

expr : tINTEGER              { $$ = new cdk::balanced3_node(LINE, *$1); delete $1; }
     | tSTRING               { $$ = new cdk::string_node(LINE, $1); }
     | '-' expr %prec tUNARY { $$ = new cdk::unary_minus_node(LINE, $2); }
     | '+' expr %prec tUNARY { $$ = new cdk::unary_plus_node(LINE, $2); }
     | expr '+' expr         { $$ = new cdk::add_node(LINE, $1, $3); }
     | expr '-' expr         { $$ = new cdk::sub_node(LINE, $1, $3); }
     | expr '*' expr         { $$ = new cdk::mul_node(LINE, $1, $3); }
     | expr '/' expr         { $$ = new cdk::div_node(LINE, $1, $3); }
     | expr '%' expr         { $$ = new cdk::mod_node(LINE, $1, $3); }
     | expr '<' expr         { $$ = new cdk::lt_node(LINE, $1, $3); }
     | expr '>' expr         { $$ = new cdk::gt_node(LINE, $1, $3); }
     | expr tGE expr         { $$ = new cdk::ge_node(LINE, $1, $3); }
     | expr tLE expr         { $$ = new cdk::le_node(LINE, $1, $3); }
     | expr tNE expr         { $$ = new cdk::ne_node(LINE, $1, $3); }
     | expr tEQ expr         { $$ = new cdk::eq_node(LINE, $1, $3); }
     | '(' expr ')'          { $$ = $2; }
     | lval                  { $$ = new cdk::rvalue_node(LINE, $1); }
     | lval '=' expr         { $$ = new cdk::assignment_node(LINE, $1, $3); }
     ;

lval : tIDENTIFIER             { $$ = new cdk::variable_node(LINE, $1); }
     ;

%%
