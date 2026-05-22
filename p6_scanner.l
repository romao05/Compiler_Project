%option c++ prefix="p6_scanner_" outfile="p6_scanner.cpp"
%option stack noyywrap yylineno 8bit debug
%{
#include <stdexcept>
#include <string>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include <cdk/types/balanced3_type.h>
#include <cdk/types/takum3_type.h>
#include "p6_parser.tab.h"

// don't change this
#define yyerror LexerError

// P6 integers are 40-trit balanced ternary: the representable range
// is +/- (3^40 - 1)/2.  Literals beyond it are a lexical error.
static const long long BALANCED3_MAX = 6078832729528464400LL;

// Parse a non-negative base-10 or base-9 integer literal; sets `overflow`
// (and returns 0) if the value does not fit the 40-trit range.
static long long p6_scan_int(const char *text, int base, bool &overflow) {
  long long v = 0;
  overflow = false;
  try {
    v = std::stoll(text, nullptr, base);
  } catch (const std::out_of_range &) {
    overflow = true;
  }
  if (overflow || v > BALANCED3_MAX) {
    overflow = true;
    return 0;
  }
  return v;
}
%}

%x X_STRING X_STRING_DONE X_STRING_SKIP X_COMMENT
%%
  yydebug=1; set_debug(1);

  /* --- Comments --- */

"//".*                          ; /* explanatory: discard until end of line */

"/*"                            yy_push_state(X_COMMENT);
<X_COMMENT>"/*"                 yy_push_state(X_COMMENT);
<X_COMMENT>"*/"                 yy_pop_state();
<X_COMMENT>.|\n                 ; /* swallow contents (including nested) */
<X_COMMENT><<EOF>>              yyerror("unterminated block comment");

  /* Multi-character operators / delimiters */

">="                            return tGE;
"<="                            return tLE;
"=="                            return tEQ;
"!="                            return tNE;
"&&"                            return tAND;
"||"                            return tOR;
"->"                            return tARROW;
"!!"                            return tPRINTLN;

  /* ==== Reserved words (per the P6 reference manual) ==== */

"int"                           return tTYPE_INT;
"real"                          return tTYPE_REAL;
"string"                        return tTYPE_STRING;
"void"                          return tTYPE_VOID;

"extern"                        return tEXTERN;
"forward"                       return tFORWARD;
"public"                        return tPUBLIC;
"auto"                          return tAUTO;

"if"                            return tIF;
"elif"                          return tELIF;
"else"                          return tELSE;
"while"                         return tWHILE;
"stop"                          return tSTOP;
"next"                          return tNEXT;
"return"                        return tRETURN;

"input"                         return tINPUT;
"null"                          return tNULL;
"sizeof"                        return tSIZEOF;

"begin"                         return tBEGIN;
"end"                           return tEND;

  /* Identifiers: letter, then letters or digits (no underscore). */
[A-Za-z][A-Za-z0-9]*            { yylval.s = new std::string(yytext); return tIDENTIFIER; }

  /*
   * Real literals (Takum3) -- written in C-style, base 10.
   *   d+ . d*  [exp]?   |   . d+ [exp]?   |   d+ exp
   * exp = [eE] [-+]? d+
   */

[0-9]+"."[0-9]*([eE][-+]?[0-9]+)?    |
"."[0-9]+([eE][-+]?[0-9]+)?          |
[0-9]+[eE][-+]?[0-9]+                {
                                       yylval.t = new cdk::takum3_type::value_type(yytext);
                                       return tREAL;
                                     }

  /* ---------------------------------------------------------
   * Integer literals (three bases; stored as balanced ternary).
   *   - base 3 balanced: "0t" followed by -, 0, +
   *   - base 9         : "0" followed by digits 0..8
   *   - decimal        : non-zero leading digit (or the lone "0")
   * "09" (and similar) is rejected as an invalid literal.
   * Values beyond the 40-trit range are a lexical error.
   * --------------------------------------------------------- */

"0t"[-0+]+                      {
                                  long long v = 0;
                                  if (yyleng - 2 > 40) {
                                    yyerror("overflow in base-3 integer literal");
                                  } else {
                                    for (const char *p = yytext + 2; *p; ++p)
                                      v = v * 3 + ((*p == '+') ? 1 : (*p == '-') ? -1 : 0);
                                  }
                                  yylval.b = new cdk::balanced3_type::value_type(v);
                                  return tINTEGER;
                                }

0[0-8]+                         {
                                  bool overflow = false;
                                  long long v = p6_scan_int(yytext, 9, overflow);
                                  if (overflow) yyerror("overflow in base-9 integer literal");
                                  yylval.b = new cdk::balanced3_type::value_type(v);
                                  return tINTEGER;
                                }

0[0-9]+                         yyerror("invalid integer literal (leading zero)");

[1-9][0-9]*                     {
                                  bool overflow = false;
                                  long long v = p6_scan_int(yytext, 10, overflow);
                                  if (overflow) yyerror("overflow in decimal integer literal");
                                  yylval.b = new cdk::balanced3_type::value_type(v);
                                  return tINTEGER;
                                }

"0"                             {
                                  yylval.b = new cdk::balanced3_type::value_type((long long) 0);
                                  return tINTEGER;
                                }


\"                              { yylval.s = new std::string(""); BEGIN(X_STRING); }

<X_STRING>\\t                   *yylval.s += '\t';
<X_STRING>\\n                   *yylval.s += '\n';
<X_STRING>\\r                   *yylval.s += '\r';
<X_STRING>\\\"                  *yylval.s += '"';
<X_STRING>\\\\                  *yylval.s += '\\';
<X_STRING>\\[0-7]{1,3}          {
                                  long c = std::stol(yytext + 1, nullptr, 8);
                                  if (c < 0 || c > 255) {
                                    yyerror("escape value out of range");
                                  } else if (c == 0) {
                                    BEGIN(X_STRING_SKIP);
                                  } else {
                                    *yylval.s += static_cast<char>(c);
                                  }
                                }
<X_STRING>\\.                   yyerror("invalid escape sequence in string");
<X_STRING>\n                    yyerror("newline in string");
<X_STRING>\"                    BEGIN(X_STRING_DONE);
<X_STRING>.                     *yylval.s += *yytext;
<X_STRING><<EOF>>               yyerror("unterminated string literal");

<X_STRING_SKIP>\\\"             ; /* swallow escaped quote */
<X_STRING_SKIP>\\.              ; /* swallow any other escape */
<X_STRING_SKIP>\n               yyerror("newline in string");
<X_STRING_SKIP>\"               BEGIN(X_STRING_DONE);
<X_STRING_SKIP>.                ; /* discard */
<X_STRING_SKIP><<EOF>>          yyerror("unterminated string literal");

<X_STRING_DONE>[ \t\r\n]+       ; /* whitespace between adjacent literals */
<X_STRING_DONE>"/*"             yy_push_state(X_COMMENT); /* comments are separators too */
<X_STRING_DONE>"//".*           ; /* explanatory comment between adjacent literals */
<X_STRING_DONE>\"               BEGIN(X_STRING);
<X_STRING_DONE><<EOF>>          BEGIN(INITIAL); return tSTRING;
<X_STRING_DONE>.                yyless(0); BEGIN(INITIAL); return tSTRING;

  /* Single-character operators / delimiters */
[-+*/%<>=?~,;!(){}\[\]]         return *yytext;

  /* --- White space --- */
[ \t\r\n]+                      ; /* ignore */

.                               yyerror("Unknown character");

%%
