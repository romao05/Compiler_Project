#include "targets/frame_size_calculator.h"
#include ".auto/all_nodes.h"

// CDK nodes – all EMPTY (not needed for size calculation)
void p6::frame_size_calculator::do_nil_node(cdk::nil_node *const, int) {}
void p6::frame_size_calculator::do_data_node(cdk::data_node *const, int) {}
void p6::frame_size_calculator::do_double_node(cdk::double_node *const, int) {}
void p6::frame_size_calculator::do_balanced3_node(cdk::balanced3_node *const, int) {}
void p6::frame_size_calculator::do_posit3_node(cdk::posit3_node *const, int) {}
void p6::frame_size_calculator::do_takum3_node(cdk::takum3_node *const, int) {}
void p6::frame_size_calculator::do_integer_node(cdk::integer_node *const, int) {}
void p6::frame_size_calculator::do_string_node(cdk::string_node *const, int) {}
void p6::frame_size_calculator::do_not_node(cdk::not_node *const, int) {}
void p6::frame_size_calculator::do_and_node(cdk::and_node *const, int) {}
void p6::frame_size_calculator::do_or_node(cdk::or_node *const, int) {}
void p6::frame_size_calculator::do_unary_minus_node(cdk::unary_minus_node *const, int) {}
void p6::frame_size_calculator::do_unary_plus_node(cdk::unary_plus_node *const, int) {}
void p6::frame_size_calculator::do_add_node(cdk::add_node *const, int) {}
void p6::frame_size_calculator::do_sub_node(cdk::sub_node *const, int) {}
void p6::frame_size_calculator::do_mul_node(cdk::mul_node *const, int) {}
void p6::frame_size_calculator::do_div_node(cdk::div_node *const, int) {}
void p6::frame_size_calculator::do_mod_node(cdk::mod_node *const, int) {}
void p6::frame_size_calculator::do_lt_node(cdk::lt_node *const, int) {}
void p6::frame_size_calculator::do_le_node(cdk::le_node *const, int) {}
void p6::frame_size_calculator::do_ge_node(cdk::ge_node *const, int) {}
void p6::frame_size_calculator::do_gt_node(cdk::gt_node *const, int) {}
void p6::frame_size_calculator::do_ne_node(cdk::ne_node *const, int) {}
void p6::frame_size_calculator::do_eq_node(cdk::eq_node *const, int) {}
void p6::frame_size_calculator::do_variable_node(cdk::variable_node *const, int) {}
void p6::frame_size_calculator::do_rvalue_node(cdk::rvalue_node *const, int) {}
void p6::frame_size_calculator::do_assignment_node(cdk::assignment_node *const, int) {}

// P6 nodes – most EMPTY
void p6::frame_size_calculator::do_program_node(p6::program_node *const, int) {}
void p6::frame_size_calculator::do_evaluation_node(p6::evaluation_node *const, int) {}
void p6::frame_size_calculator::do_write_node(p6::write_node *const, int) {}
void p6::frame_size_calculator::do_input_node(p6::input_node *const, int) {}
void p6::frame_size_calculator::do_null_node(p6::null_node *const, int) {}
void p6::frame_size_calculator::do_sizeof_node(p6::sizeof_node *const, int) {}
void p6::frame_size_calculator::do_address_of_node(p6::address_of_node *const, int) {}
void p6::frame_size_calculator::do_index_node(p6::index_node *const, int) {}
void p6::frame_size_calculator::do_stack_alloc_node(p6::stack_alloc_node *const, int) {}
void p6::frame_size_calculator::do_return_node(p6::return_node *const, int) {}
void p6::frame_size_calculator::do_stop_node(p6::stop_node *const, int) {}
void p6::frame_size_calculator::do_next_node(p6::next_node *const, int) {}
void p6::frame_size_calculator::do_function_declaration_node(p6::function_declaration_node *const, int) {}
void p6::frame_size_calculator::do_function_call_node(p6::function_call_node *const, int) {}

// Structural nodes – traverse to find variable declarations

void p6::frame_size_calculator::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++)
    node->node(i)->accept(this, lvl);
}

void p6::frame_size_calculator::do_function_definition_node(p6::function_definition_node *const node, int lvl) {
  // only visit the body; arguments are at positive offsets and don't count
  node->block()->accept(this, lvl);
}

void p6::frame_size_calculator::do_block_node(p6::block_node *const node, int lvl) {
  if (node->declarations()) node->declarations()->accept(this, lvl + 2);
  if (node->instructions()) node->instructions()->accept(this, lvl + 2);
}

void p6::frame_size_calculator::do_variable_declaration_node(p6::variable_declaration_node *const node, int lvl) {
  // skip extern/forward – no stack space needed
  if (node->qualifier() == QUALIFIER_EXTERN || node->qualifier() == QUALIFIER_FORWARD)
    return;
  _localsize += node->type()->size();
}

void p6::frame_size_calculator::do_if_node(p6::if_node *const node, int lvl) {
  node->block()->accept(this, lvl);
}

void p6::frame_size_calculator::do_if_else_node(p6::if_else_node *const node, int lvl) {
  node->thenblock()->accept(this, lvl);
  node->elseblock()->accept(this, lvl);
}

void p6::frame_size_calculator::do_while_node(p6::while_node *const node, int lvl) {
  node->block()->accept(this, lvl);
}

// The iteration keeps its state (pointer + counter) on the operand stack and
// declares no locals, so it contributes nothing to the frame size.
void p6::frame_size_calculator::do_unless_iterate_node(p6::unless_iterate_node *const, int) {}
