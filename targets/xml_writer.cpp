#include <string>
#include "targets/xml_writer.h"
#include "targets/type_checker.h"
#include ".auto/all_nodes.h"  // automatically generated

//---------------------------------------------------------------------------

void p6::xml_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
  openTag(node, lvl);
  closeTag(node, lvl);
}
void p6::xml_writer::do_data_node(cdk::data_node * const node, int lvl) {
  openTag(node, lvl);
  closeTag(node, lvl);
}
void p6::xml_writer::do_double_node(cdk::double_node * const node, int lvl) {
  process_literal(node, lvl);
}
void p6::xml_writer::do_balanced3_node(cdk::balanced3_node * const node, int lvl) {
  os() << std::string(lvl, ' ') << '<' << node->label() << '>' << node->value().to_string() << "</" << node->label() << '>' << std::endl;
}
void p6::xml_writer::do_posit3_node(cdk::posit3_node * const node, int lvl) {
  os() << std::string(lvl, ' ') << '<' << node->label() << '>' << node->value().to_string() << "</" << node->label() << '>' << std::endl;
}
void p6::xml_writer::do_takum3_node(cdk::takum3_node * const node, int lvl) {
  os() << std::string(lvl, ' ') << '<' << node->label() << '>' << node->value().to_string() << "</" << node->label() << '>' << std::endl;
}
void p6::xml_writer::do_not_node(cdk::not_node * const node, int lvl) {
  do_unary_operation(node, lvl);
}
void p6::xml_writer::do_and_node(cdk::and_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void p6::xml_writer::do_or_node(cdk::or_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}

//---------------------------------------------------------------------------

void p6::xml_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  os() << std::string(lvl, ' ') << "<sequence_node size='" << node->size() << "'>" << std::endl;
  for (size_t i = 0; i < node->size(); ++i) {
    openTag("arg", lvl);
    node->node(i)->accept(this, lvl + 2);
    closeTag("arg", lvl);
  }
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void p6::xml_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  process_literal(node, lvl);
}

void p6::xml_writer::do_string_node(cdk::string_node * const node, int lvl) {
  process_literal(node, lvl);
}

//---------------------------------------------------------------------------

void p6::xml_writer::do_unary_operation(cdk::unary_operation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  node->argument()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

void p6::xml_writer::do_unary_minus_node(cdk::unary_minus_node * const node, int lvl) {
  do_unary_operation(node, lvl);
}

void p6::xml_writer::do_unary_plus_node(cdk::unary_plus_node * const node, int lvl) {
  do_unary_operation(node, lvl);
}

//---------------------------------------------------------------------------

void p6::xml_writer::do_binary_operation(cdk::binary_operation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  openTag("left", lvl);
  node->left()->accept(this, lvl + 2);
  closeTag("left", lvl);
  openTag("right", lvl);
  node->right()->accept(this, lvl + 2);
  closeTag("right", lvl);
  closeTag(node, lvl);
}

void p6::xml_writer::do_add_node(cdk::add_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void p6::xml_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void p6::xml_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void p6::xml_writer::do_div_node(cdk::div_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void p6::xml_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void p6::xml_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void p6::xml_writer::do_le_node(cdk::le_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void p6::xml_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void p6::xml_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void p6::xml_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void p6::xml_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}

//---------------------------------------------------------------------------

void p6::xml_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  os() << std::string(lvl, ' ') << "<" << node->label() << ">" << node->name() << "</" << node->label() << ">" << std::endl;
}

void p6::xml_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  node->lvalue()->accept(this, lvl + 4);
  closeTag(node, lvl);
}

void p6::xml_writer::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);

  openTag("lvalue", lvl);
  node->lvalue()->accept(this, lvl + 2);
  closeTag("lvalue", lvl);
  reset_new_symbol();

  openTag("rvalue", lvl);
  node->rvalue()->accept(this, lvl + 2);
  closeTag("rvalue", lvl);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void p6::xml_writer::do_program_node(p6::program_node * const node, int lvl) {
  openTag(node, lvl);
  node->block()->accept(this, lvl + 4);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void p6::xml_writer::do_evaluation_node(p6::evaluation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  node->argument()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

void p6::xml_writer::do_write_node(p6::write_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  node->arguments()->accept(this, lvl + 2);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void p6::xml_writer::do_input_node(p6::input_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void p6::xml_writer::do_block_node(p6::block_node * const node, int lvl) {
  openTag(node, lvl);
  openTag("declarations", lvl + 2);
  if (node->declarations()) node->declarations()->accept(this, lvl + 4);
  closeTag("declarations", lvl + 2);
  openTag("instructions", lvl + 2);
  if (node->instructions()) node->instructions()->accept(this, lvl + 4);
  closeTag("instructions", lvl + 2);
  closeTag(node, lvl);
}

void p6::xml_writer::do_variable_declaration_node(p6::variable_declaration_node * const node, int lvl) {
  os() << std::string(lvl, ' ') << "<" << node->label()
       << " qualifier='" << node->qualifier()
       << "' identifier='" << node->identifier() << "'>" << std::endl;
  if (node->initializer()) {
    openTag("initializer", lvl + 2);
    node->initializer()->accept(this, lvl + 4);
    closeTag("initializer", lvl + 2);
  }
  closeTag(node, lvl);
}

void p6::xml_writer::do_function_definition_node(p6::function_definition_node * const node, int lvl) {
  os() << std::string(lvl, ' ') << "<" << node->label()
       << " qualifier='" << node->qualifier()
       << "' identifier='" << node->identifier() << "'>" << std::endl;
  openTag("arguments", lvl + 2);
  if (node->arguments()) node->arguments()->accept(this, lvl + 4);
  closeTag("arguments", lvl + 2);
  openTag("block", lvl + 2);
  if (node->block()) node->block()->accept(this, lvl + 4);
  closeTag("block", lvl + 2);
  closeTag(node, lvl);
}

void p6::xml_writer::do_function_declaration_node(p6::function_declaration_node * const node, int lvl) {
  os() << std::string(lvl, ' ') << "<" << node->label()
       << " qualifier='" << node->qualifier()
       << "' identifier='" << node->identifier() << "'>" << std::endl;
  openTag("arguments", lvl + 2);
  if (node->arguments()) node->arguments()->accept(this, lvl + 4);
  closeTag("arguments", lvl + 2);
  closeTag(node, lvl);
}

void p6::xml_writer::do_function_call_node(p6::function_call_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  os() << std::string(lvl, ' ') << "<" << node->label()
       << " identifier='" << node->identifier() << "'>" << std::endl;
  openTag("arguments", lvl + 2);
  if (node->arguments()) node->arguments()->accept(this, lvl + 4);
  closeTag("arguments", lvl + 2);
  closeTag(node, lvl);
}

void p6::xml_writer::do_null_node(p6::null_node * const node, int lvl) {
  openTag(node, lvl);
  closeTag(node, lvl);
}

void p6::xml_writer::do_sizeof_node(p6::sizeof_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  openTag("expression", lvl + 2);
  node->expression()->accept(this, lvl + 4);
  closeTag("expression", lvl + 2);
  closeTag(node, lvl);
}

void p6::xml_writer::do_address_of_node(p6::address_of_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  openTag("lvalue", lvl + 2);
  node->lvalue()->accept(this, lvl + 4);
  closeTag("lvalue", lvl + 2);
  closeTag(node, lvl);
}

void p6::xml_writer::do_index_node(p6::index_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  openTag("base", lvl + 2);
  node->base()->accept(this, lvl + 4);
  closeTag("base", lvl + 2);
  openTag("index", lvl + 2);
  node->index()->accept(this, lvl + 4);
  closeTag("index", lvl + 2);
  closeTag(node, lvl);
}

void p6::xml_writer::do_stack_alloc_node(p6::stack_alloc_node * const node, int lvl) {
  do_unary_operation(node, lvl);
}

void p6::xml_writer::do_return_node(p6::return_node * const node, int lvl) {
  openTag(node, lvl);
  if (node->expression()) {
    openTag("expression", lvl + 2);
    node->expression()->accept(this, lvl + 4);
    closeTag("expression", lvl + 2);
  }
  closeTag(node, lvl);
}

void p6::xml_writer::do_stop_node(p6::stop_node * const node, int lvl) {
  os() << std::string(lvl, ' ') << "<" << node->label()
       << " level='" << node->level() << "'/>" << std::endl;
}

void p6::xml_writer::do_next_node(p6::next_node * const node, int lvl) {
  os() << std::string(lvl, ' ') << "<" << node->label()
       << " level='" << node->level() << "'/>" << std::endl;
}

//---------------------------------------------------------------------------

void p6::xml_writer::do_while_node(p6::while_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  openTag("condition", lvl + 2);
  node->condition()->accept(this, lvl + 4);
  closeTag("condition", lvl + 2);
  openTag("block", lvl + 2);
  node->block()->accept(this, lvl + 4);
  closeTag("block", lvl + 2);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void p6::xml_writer::do_if_node(p6::if_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  openTag("condition", lvl + 2);
  node->condition()->accept(this, lvl + 4);
  closeTag("condition", lvl + 2);
  openTag("then", lvl + 2);
  node->block()->accept(this, lvl + 4);
  closeTag("then", lvl + 2);
  closeTag(node, lvl);
}

void p6::xml_writer::do_if_else_node(p6::if_else_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  openTag("condition", lvl + 2);
  node->condition()->accept(this, lvl + 4);
  closeTag("condition", lvl + 2);
  openTag("then", lvl + 2);
  node->thenblock()->accept(this, lvl + 4);
  closeTag("then", lvl + 2);
  openTag("else", lvl + 2);
  node->elseblock()->accept(this, lvl + 4);
  closeTag("else", lvl + 2);
  closeTag(node, lvl);
}
