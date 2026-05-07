#pragma once

#include <string>
#include <cdk/ast/typed_node.h>
#include <cdk/ast/sequence_node.h>

namespace p6 {

  /**
   * Class for describing function declarations (forward/extern, no body).
   */
  class function_declaration_node : public cdk::typed_node {
    int _qualifier;
    std::string _identifier;
    cdk::sequence_node *_arguments;

  public:
    function_declaration_node(int lineno, int qualifier, std::shared_ptr<cdk::basic_type> funType,
                              const std::string &identifier, cdk::sequence_node *arguments) :
        cdk::typed_node(lineno), _qualifier(qualifier), _identifier(identifier), _arguments(arguments) {
      type(funType);
    }

    int qualifier() { return _qualifier; }
    const std::string &identifier() const { return _identifier; }
    cdk::sequence_node *arguments() { return _arguments; }
    cdk::typed_node *argument(size_t ax) {
      return dynamic_cast<cdk::typed_node *>(_arguments->node(ax));
    }

    void accept(basic_ast_visitor *sp, int level) { sp->do_function_declaration_node(this, level); }
  };

} // p6
