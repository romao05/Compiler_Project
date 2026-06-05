#pragma once

#include "targets/basic_ast_visitor.h"

#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <cdk/emitters/basic_postfix_emitter.h>

namespace p6 {

  //!
  //! Traverse syntax tree and generate the corresponding assembly code.
  //!
  class postfix_writer: public basic_ast_visitor {
    cdk::symbol_table<p6::symbol> &_symtab;
    cdk::basic_postfix_emitter &_pf;
    int _lbl;
    int _offset;
    bool _inFunctionBody;
    bool _inFunctionArgs;
    std::shared_ptr<p6::symbol> _function; // symbol of the function being generated
    int _funcEndLabel;                     // label of the current function epilogue
    int _funcSretOffset;                   // frame offset of the hidden takum3-return pointer
    std::vector<int> _whileCond;           // stack of loop condition labels (for 'next')
    std::vector<int> _whileEnd;            // stack of loop end labels (for 'stop')
    std::set<std::string> _forwardFunctions; // names declared 'forward' in this module
    std::set<std::string> _definedFunctions; // names of functions defined in this module
    std::map<std::string, std::vector<std::shared_ptr<cdk::basic_type>>> _funcArgTypes; // formal parameter types per function (for implicit arg conversions)

  public:
    postfix_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<p6::symbol> &symtab,
                   cdk::basic_postfix_emitter &pf) :
        basic_ast_visitor(compiler), _symtab(symtab), _pf(pf), _lbl(0),
        _offset(0), _inFunctionBody(false), _inFunctionArgs(false),
        _function(nullptr), _funcEndLabel(0), _funcSretOffset(0) {
    }

  public:
    ~postfix_writer() {
      os().flush();
    }

  private:
    /** Method used to generate sequential labels. */
    inline std::string mklbl(int lbl) {
      std::ostringstream oss;
      if (lbl < 0)
        oss << ".L" << -lbl;
      else
        oss << "_L" << lbl;
      return oss.str();
    }

    /** Map an int 0/1 boolean (on top of the stack) into a balanced3 ternary
        boolean: false 0 -> -1, true 1 -> +1 (i.e. 2*x - 1, then int -> balanced3). */
    inline void boolToBalanced3() {
      _pf.DUP32();
      _pf.ADD();
      _pf.INT(1);
      _pf.SUB();
      _pf.I2B();
    }

  public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end

  };

} // p6

