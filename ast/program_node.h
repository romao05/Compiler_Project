#pragma once

#include <cdk/ast/basic_node.h>
#include "ast/block_node.h"

namespace p6 {

  /**
   * Class for describing program nodes (begin <decls> <stmts> end).
   */
  class program_node : public cdk::basic_node {
    p6::block_node *_block;

  public:
    program_node(int lineno, p6::block_node *block) :
        cdk::basic_node(lineno), _block(block) {
    }

    p6::block_node *block() { return _block; }

    void accept(basic_ast_visitor *sp, int level) { sp->do_program_node(this, level); }

  };

} // p6
