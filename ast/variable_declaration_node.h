#pragma once

#include <string>
#include <cdk/ast/typed_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/types/basic_type.h>

namespace p6 {

  /**
   * Storage-class qualifiers applicable to P6 declarations (variables and
   * functions).  `private` is the default and has no keyword in the language,
   * so it is NOT a lexical token; the others map to the keywords
   * public / forward / extern (see the reference manual, "Símbolos globais").
   */
  enum qualifier {
    QUALIFIER_PRIVATE = 0,  //!< default: visible only inside its own module
    QUALIFIER_PUBLIC,       //!< public:  exported to other modules
    QUALIFIER_FORWARD,      //!< forward: defined in another module
    QUALIFIER_EXTERN        //!< extern:  non-P6 (e.g. C) function symbol
  };

  /**
   * Class for describing variable declarations.
   * Supports qualifiers public/forward/extern (and "auto" inferred type).
   */
  class variable_declaration_node : public cdk::typed_node {
    int _qualifier;
    std::string _identifier;
    cdk::expression_node *_initializer;

  public:
    variable_declaration_node(int lineno, int qualifier, std::shared_ptr<cdk::basic_type> varType,
                              const std::string &identifier, cdk::expression_node *initializer) :
        cdk::typed_node(lineno), _qualifier(qualifier), _identifier(identifier), _initializer(initializer) {
      type(varType);
    }

    int qualifier() { return _qualifier; }
    const std::string &identifier() const { return _identifier; }
    cdk::expression_node *initializer() { return _initializer; }
    void initializer(cdk::expression_node *initializer) { _initializer = initializer; }

    void accept(basic_ast_visitor *sp, int level) { sp->do_variable_declaration_node(this, level); }
  };

} // p6
