#pragma once

#include <cdk/ast/expression_node.h>

#include <string>

namespace p6 {

  /**
   * Class for describing while-cycle nodes.
   */
  class between_node : public cdk::basic_node {
    cdk::expression_node *_low;
    cdk::expression_node *_high;
    std::string _function;
    cdk::expression_node *_vector;

  public:
    between_node(int lineno, cdk::expression_node *low, cdk::expression_node *high, const std::string &function, cdk::expression_node *vector) :
        basic_node(lineno), _low(low), _high(high), _function(function), _vector(vector) {
    }

    cdk::expression_node *low() { return _low; }

    cdk::expression_node *high() { return _high; }

    const std::string &function() { return _function;}

    cdk::expression_node *vector() { return _vector; }

    void accept(basic_ast_visitor *sp, int level) { sp->do_between_node(this, level); }

  };

} // p6
