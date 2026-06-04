#pragma once

#include "targets/basic_ast_visitor.h"

namespace p6 {

  class frame_size_calculator : public basic_ast_visitor {
    int _localsize;

  public:
    frame_size_calculator(std::shared_ptr<cdk::compiler> compiler) :
        basic_ast_visitor(compiler), _localsize(0) {
    }

    int localsize() const { return _localsize; }

  public:
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"
#undef __IN_VISITOR_HEADER__
  };

} // p6
