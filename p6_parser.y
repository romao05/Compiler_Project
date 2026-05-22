%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <algorithm>
#include <memory>
#include <vector>
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

  int                   i;          /* qualifier (public/forward/extern/private) */
  cdk::balanced3_type::value_type *b; /* P6 integer literal (balanced ternary) */
  cdk::takum3_type::value_type    *t; /* P6 real literal (Takum3) */
  std::string          *s;          /* symbol name or string literal */
  cdk::basic_node      *node;       /* node pointer */
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;
  p6::block_node       *block;
  std::vector<std::shared_ptr<cdk::basic_type>> *types;
};

%token <b> tINTEGER
%token <t> tREAL
%token <s> tIDENTIFIER tSTRING
%token tBEGIN tEND
%token tIF tELIF tELSE tWHILE tSTOP tNEXT tRETURN
%token tINPUT tNULL tSIZEOF
%token tTYPE_INT tTYPE_REAL tTYPE_STRING tTYPE_VOID
%token tEXTERN tFORWARD tPUBLIC tPRIVATE tAUTO
%token tARROW tPRINTLN

%nonassoc tIFX
%nonassoc tELIF tELSE

%right '='
%left tOR
%left tAND
%left tEQ tNE
%left '<' '>' tLE tGE
%left '+' '-'
%left '*' '/' '%'
%nonassoc '~'
%nonassoc tUNARY

%type <node> declaracao programa instr elifs variavel
%type <sequence> decls variaveis instrs exprs
%type <i> qualif optqualif
%type <type> tipo
%type <types> tipos
%type <block> bloco
%type <expression> expr
%type <lvalue> lval

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

ficheiro : decls           { compiler->ast($1); }
         | decls programa  { compiler->ast(new cdk::sequence_node(LINE, $2, $1)); }
         ;

decls :                  { $$ = new cdk::sequence_node(LINE); }
      | decls declaracao { $$ = new cdk::sequence_node(LINE, $2, $1); }
      ;

programa : tBEGIN decls instrs tEND { $$ = new p6::program_node(LINE, new p6::block_node(LINE, $2, $3)); }
         ;

qualif : tPUBLIC   { $$ = tPUBLIC; }
       | tFORWARD  { $$ = tFORWARD; }
       | tEXTERN   { $$ = tEXTERN; }
       ;

optqualif :         { $$ = tPRIVATE; }
          | qualif  { $$ = $1; }
          ;

declaracao : optqualif tipo tIDENTIFIER ';'
               { if (cdk::functional_type::cast($2))
                   $$ = new p6::function_declaration_node(LINE, $1, $2, *$3, new cdk::sequence_node(LINE));
                 else
                   $$ = new p6::variable_declaration_node(LINE, $1, $2, *$3, nullptr);
                 delete $3; }
           | optqualif tipo tIDENTIFIER '=' expr ';'
               { $$ = new p6::variable_declaration_node(LINE, $1, $2, *$3, $5); delete $3; }
           | optqualif tAUTO tIDENTIFIER '=' expr ';'
               { $$ = new p6::variable_declaration_node(LINE, $1, nullptr, *$3, $5); delete $3; }
           | qualif tIDENTIFIER '=' expr ';'
               { $$ = new p6::variable_declaration_node(LINE, $1, nullptr, *$2, $4); delete $2; }
           | optqualif tIDENTIFIER '(' ')' tARROW tipo bloco
               { $$ = new p6::function_definition_node(LINE, $1, $6, *$2, new cdk::sequence_node(LINE), $7); delete $2; }
           | optqualif tIDENTIFIER '(' variaveis ')' tARROW tipo bloco
               { $$ = new p6::function_definition_node(LINE, $1, $7, *$2, $4, $8); delete $2; }
           ;

variaveis : variavel               { $$ = new cdk::sequence_node(LINE, $1); }
          | variaveis ',' variavel { $$ = new cdk::sequence_node(LINE, $3, $1); }
          ;

variavel : tipo tIDENTIFIER { $$ = new p6::variable_declaration_node(LINE, tPRIVATE, $1, *$2, nullptr); delete $2; }
         ;

tipo : tTYPE_INT          { $$ = cdk::balanced3_type::create(); }
     | tTYPE_REAL         { $$ = cdk::takum3_type::create(); }
     | tTYPE_STRING       { $$ = cdk::primitive_type::create(4, cdk::TYPE_STRING); }
     | tTYPE_VOID         { $$ = cdk::primitive_type::create(0, cdk::TYPE_VOID); }
     | '[' tipo ']'       { $$ = cdk::reference_type::create(4, $2); }
     | tipo '<' '>'       { $$ = cdk::functional_type::create($1); }
     | tipo '<' tipos '>' { $$ = cdk::functional_type::create(*$3, $1); delete $3; }
     ;

tipos : tipo           { $$ = new std::vector<std::shared_ptr<cdk::basic_type>>(); $$->push_back($1); }
      | tipos ',' tipo { $$ = $1; $1->push_back($3); }
      ;

instrs :                { $$ = new cdk::sequence_node(LINE); }
       | instrs instr   { $$ = new cdk::sequence_node(LINE, $2, $1); }
       ;

instr : expr ';'                            { $$ = new p6::evaluation_node(LINE, $1); }
      | exprs '!'                           { $$ = new p6::write_node(LINE, $1, false); }
      | exprs tPRINTLN                      { $$ = new p6::write_node(LINE, $1, true); }
      | tSTOP ';'                           { $$ = new p6::stop_node(LINE); }
      | tSTOP tINTEGER ';'                  { $$ = new p6::stop_node(LINE, $2->to_int()); delete $2; }
      | tNEXT ';'                           { $$ = new p6::next_node(LINE); }
      | tNEXT tINTEGER ';'                  { $$ = new p6::next_node(LINE, $2->to_int()); delete $2; }
      | tRETURN ';'                         { $$ = new p6::return_node(LINE); }
      | tRETURN expr ';'                    { $$ = new p6::return_node(LINE, $2); }
      | tWHILE '(' expr ')' instr           { $$ = new p6::while_node(LINE, $3, $5); }
      | tIF '(' expr ')' instr %prec tIFX   { $$ = new p6::if_node(LINE, $3, $5); }
      | tIF '(' expr ')' instr tELSE instr  { $$ = new p6::if_else_node(LINE, $3, $5, $7); }
      | tIF '(' expr ')' instr elifs        { $$ = new p6::if_else_node(LINE, $3, $5, $6); }
      | bloco                               { $$ = $1; }
      ;

elifs : tELIF '(' expr ')' instr %prec tIFX   { $$ = new p6::if_node(LINE, $3, $5); }
      | tELIF '(' expr ')' instr tELSE instr  { $$ = new p6::if_else_node(LINE, $3, $5, $7); }
      | tELIF '(' expr ')' instr elifs        { $$ = new p6::if_else_node(LINE, $3, $5, $6); }
      ;

bloco : '{' decls instrs '}'  { $$ = new p6::block_node(LINE, $2, $3); }
      ;

exprs : expr            { $$ = new cdk::sequence_node(LINE, $1); }
      | exprs ',' expr  { $$ = new cdk::sequence_node(LINE, $3, $1); }
      ;

expr : tINTEGER                  { $$ = new cdk::balanced3_node(LINE, *$1); delete $1; }
     | tREAL                     { $$ = new cdk::takum3_node(LINE, *$1); delete $1; }
     | tSTRING                   { $$ = new cdk::string_node(LINE, $1); }
     | tNULL                     { $$ = new p6::null_node(LINE); }
     | tINPUT                    { $$ = new p6::input_node(LINE); }
     | '+' expr %prec tUNARY     { $$ = new cdk::unary_plus_node(LINE, $2); }
     | '-' expr %prec tUNARY     { $$ = new cdk::unary_minus_node(LINE, $2); }
     | '~' expr                  { $$ = new cdk::not_node(LINE, $2); }
     | expr '+' expr             { $$ = new cdk::add_node(LINE, $1, $3); }
     | expr '-' expr             { $$ = new cdk::sub_node(LINE, $1, $3); }
     | expr '*' expr             { $$ = new cdk::mul_node(LINE, $1, $3); }
     | expr '/' expr             { $$ = new cdk::div_node(LINE, $1, $3); }
     | expr '%' expr             { $$ = new cdk::mod_node(LINE, $1, $3); }
     | expr '<' expr             { $$ = new cdk::lt_node(LINE, $1, $3); }
     | expr '>' expr             { $$ = new cdk::gt_node(LINE, $1, $3); }
     | expr tGE expr             { $$ = new cdk::ge_node(LINE, $1, $3); }
     | expr tLE expr             { $$ = new cdk::le_node(LINE, $1, $3); }
     | expr tEQ expr             { $$ = new cdk::eq_node(LINE, $1, $3); }
     | expr tNE expr             { $$ = new cdk::ne_node(LINE, $1, $3); }
     | expr tAND expr            { $$ = new cdk::and_node(LINE, $1, $3); }
     | expr tOR expr             { $$ = new cdk::or_node(LINE, $1, $3); }
     | '(' expr ')'              { $$ = $2; }
     | '[' expr ']'              { $$ = new p6::stack_alloc_node(LINE, $2); }
     | tSIZEOF '(' expr ')'      { $$ = new p6::sizeof_node(LINE, $3); }
     | tIDENTIFIER '(' ')'       { $$ = new p6::function_call_node(LINE, *$1); delete $1; }
     | tIDENTIFIER '(' exprs ')' { $$ = new p6::function_call_node(LINE, *$1, $3); delete $1; }
     | lval '?'                  { $$ = new p6::address_of_node(LINE, $1); }
     | lval                      { $$ = new cdk::rvalue_node(LINE, $1); }
     | lval '=' expr             { $$ = new cdk::assignment_node(LINE, $1, $3); }
     ;

lval : tIDENTIFIER       { $$ = new cdk::variable_node(LINE, $1); }
     | expr '[' expr ']' { $$ = new p6::index_node(LINE, $1, $3); }
     ;

%%
