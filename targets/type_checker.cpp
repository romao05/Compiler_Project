#include <string>
#include <memory>
#include "targets/type_checker.h"
#include ".auto/all_nodes.h" // automatically generated
#include <cdk/types/primitive_type.h>
#include <cdk/types/reference_type.h>

//---------------------------------------------------------------------------
// P6 type checker.
//
// This pass annotates every expression node of the syntax tree with a type,
// so that ASSERT_SAFE_EXPRESSIONS (run by the other visitors) always sees a
// fully typed tree.  It is intentionally lenient: it infers a type for every
// construct and never rejects a program.  Full semantic validation (operand
// compatibility, implicit conversions, etc.) is left for the final delivery.
//---------------------------------------------------------------------------

#define ASSERT_UNSPEC                                                 \
  {                                                                   \
    if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) \
      return;                                                         \
  }

// --- type constructors (sizes follow the P6 reference manual) ---------------

static std::shared_ptr<cdk::basic_type> int_type()
{
  return cdk::primitive_type::create(8, cdk::TYPE_BALANCED3); // 40 trits / 64 bits
}
static std::shared_ptr<cdk::basic_type> real_type()
{
  return cdk::primitive_type::create(16, cdk::TYPE_TAKUM3); // 80 trits / 128 bits
}
static std::shared_ptr<cdk::basic_type> string_type()
{
  return cdk::primitive_type::create(4, cdk::TYPE_STRING);
}
static std::shared_ptr<cdk::basic_type> void_type()
{
  return cdk::primitive_type::create(0, cdk::TYPE_VOID);
}
static std::shared_ptr<cdk::basic_type> pointer_type(std::shared_ptr<cdk::basic_type> referenced)
{
  return cdk::reference_type::create(4, referenced ? referenced : void_type());
}

static bool is_real(std::shared_ptr<cdk::basic_type> t)
{
  return t != nullptr && t->name() == cdk::TYPE_TAKUM3;
}
static bool is_pointer(std::shared_ptr<cdk::basic_type> t)
{
  return t != nullptr && t->name() == cdk::TYPE_POINTER;
}

//---------------------------------------------------------------------------

void p6::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl)
{
  for (size_t i = 0; i < node->size(); ++i)
    if (node->node(i))
      node->node(i)->accept(this, lvl);
}

//---------------------------------------------------------------------------

void p6::type_checker::do_nil_node(cdk::nil_node *const node, int lvl)
{
  // EMPTY (not used by P6)
}
void p6::type_checker::do_data_node(cdk::data_node *const node, int lvl)
{
  // EMPTY (not used by P6)
}

//---------------------------------------------------------------------------
// Literals.

void p6::type_checker::do_double_node(cdk::double_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->type(real_type());
}
void p6::type_checker::do_balanced3_node(cdk::balanced3_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->type(int_type());
}
void p6::type_checker::do_posit3_node(cdk::posit3_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(16, cdk::TYPE_POSIT3));
}
void p6::type_checker::do_takum3_node(cdk::takum3_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->type(real_type());
}
void p6::type_checker::do_integer_node(cdk::integer_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->type(int_type());
}
void p6::type_checker::do_string_node(cdk::string_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->type(string_type());
}

//---------------------------------------------------------------------------
// Logical operators (Kleene tri-valued logic -- yield an integer).

void p6::type_checker::do_not_node(cdk::not_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  node->type(int_type());
}
void p6::type_checker::do_and_node(cdk::and_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);
  node->type(int_type());
}
void p6::type_checker::do_or_node(cdk::or_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);
  node->type(int_type());
}

//---------------------------------------------------------------------------
// Unary identity / symmetric.

void p6::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  node->type(is_real(node->argument()->type()) ? real_type() : int_type());
}

void p6::type_checker::do_unary_minus_node(cdk::unary_minus_node *const node, int lvl)
{
  processUnaryExpression(node, lvl);
}
void p6::type_checker::do_unary_plus_node(cdk::unary_plus_node *const node, int lvl)
{
  processUnaryExpression(node, lvl);
}

//---------------------------------------------------------------------------
// Binary arithmetic and comparisons.

void p6::type_checker::processBinaryExpression(cdk::binary_operation_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);
  auto lt = node->left()->type();
  auto rt = node->right()->type();
  if (is_pointer(lt))
    node->type(lt); // pointer offset
  else if (is_pointer(rt))
    node->type(rt);
  else if (is_real(lt) || is_real(rt))
    node->type(real_type());
  else
    node->type(int_type());
}

void p6::type_checker::processComparisonExpression(cdk::binary_operation_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);
  node->type(int_type());
}

void p6::type_checker::do_add_node(cdk::add_node *const node, int lvl)
{
  processBinaryExpression(node, lvl);
}
void p6::type_checker::do_sub_node(cdk::sub_node *const node, int lvl)
{
  processBinaryExpression(node, lvl);
}
void p6::type_checker::do_mul_node(cdk::mul_node *const node, int lvl)
{
  processBinaryExpression(node, lvl);
}
void p6::type_checker::do_div_node(cdk::div_node *const node, int lvl)
{
  processBinaryExpression(node, lvl);
}
void p6::type_checker::do_mod_node(cdk::mod_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);
  node->type(int_type()); // modulus is integer only (represented by balanced3)
}
void p6::type_checker::do_lt_node(cdk::lt_node *const node, int lvl)
{
  processComparisonExpression(node, lvl);
}
void p6::type_checker::do_le_node(cdk::le_node *const node, int lvl)
{
  processComparisonExpression(node, lvl);
}
void p6::type_checker::do_ge_node(cdk::ge_node *const node, int lvl)
{
  processComparisonExpression(node, lvl);
}
void p6::type_checker::do_gt_node(cdk::gt_node *const node, int lvl)
{
  processComparisonExpression(node, lvl);
}
void p6::type_checker::do_ne_node(cdk::ne_node *const node, int lvl)
{
  processComparisonExpression(node, lvl);
}
void p6::type_checker::do_eq_node(cdk::eq_node *const node, int lvl)
{
  processComparisonExpression(node, lvl);
}

//---------------------------------------------------------------------------
// Left-values and assignment.

void p6::type_checker::do_variable_node(cdk::variable_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  auto symbol = _symtab.find(id);
  if (symbol != nullptr) {
    node->type(symbol->type());
  } else {
    throw id; // apanhado em do_rvalue_node / do_assignment_node
  }
}

void p6::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  try {
    node->lvalue()->accept(this, lvl);
    node->type(node->lvalue()->type());
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
}

void p6::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  try {
    node->lvalue()->accept(this, lvl);
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
  node->rvalue()->accept(this, lvl);

  auto l_type = node->lvalue()->type();
  auto r_type = node->rvalue()->type();

if (is_pointer(l_type) && is_pointer(r_type)) {
  // compara os tipos referenciados recursivamente, ponteiros devem ser do mesmo tipo ou ambos void
  if (cdk::reference_type::cast(l_type)->referenced()->name()
      != cdk::reference_type::cast(r_type)->referenced()->name())
    throw std::string("incompatible pointer types in assignment");
  node->type(l_type);
} else if (l_type->name() == r_type->name()) {
  node->type(l_type);
} else if (is_real(l_type) && r_type->name() == cdk::TYPE_BALANCED3) {
  node->type(l_type);
} else {
  throw std::string("incompatible types in assignment");
}
}

//---------------------------------------------------------------------------
// Other expressions.

void p6::type_checker::do_index_node(p6::index_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->base()->accept(this, lvl + 2);
  node->index()->accept(this, lvl + 2);
  auto ref = cdk::reference_type::cast(node->base()->type());
  node->type(ref ? ref->referenced() : int_type());
}

void p6::type_checker::do_address_of_node(p6::address_of_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl + 2);
  node->type(pointer_type(node->lvalue()->type()));
}

void p6::type_checker::do_stack_alloc_node(p6::stack_alloc_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  node->type(pointer_type(void_type()));
}

//---------------------------------------------------------------------------
void p6::type_checker::do_sizeof_node(p6::sizeof_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->expression()->accept(this, lvl + 2);
  node->type(int_type());
}

void p6::type_checker::do_input_node(p6::input_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->type(int_type()); // input yields an integer unless context asks otherwise
}

void p6::type_checker::do_null_node(p6::null_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  node->type(pointer_type(void_type()));
}

void p6::type_checker::do_function_call_node(p6::function_call_node *const node, int lvl)
{
  ASSERT_UNSPEC;
  if (node->arguments())
    node->arguments()->accept(this, lvl + 2);
  auto symbol = _symtab.find(node->identifier());
  node->type(symbol && symbol->type() ? symbol->type() : int_type());
}

//---------------------------------------------------------------------------
// Instructions and structural nodes (no type of their own).

void p6::type_checker::do_evaluation_node(p6::evaluation_node *const node, int lvl)
{
  node->argument()->accept(this, lvl + 2);
}

void p6::type_checker::do_write_node(p6::write_node *const node, int lvl)
{
  node->arguments()->accept(this, lvl + 2);
}

void p6::type_checker::do_return_node(p6::return_node *const node, int lvl)
{
  if (node->expression())
    node->expression()->accept(this, lvl + 2);
}

void p6::type_checker::do_stop_node(p6::stop_node *const node, int lvl)
{
  // EMPTY (no expressions to check)
}

void p6::type_checker::do_next_node(p6::next_node *const node, int lvl)
{
  // EMPTY (no expressions to check)
}

void p6::type_checker::do_while_node(p6::while_node *const node, int lvl)
{
  node->condition()->accept(this, lvl + 4);
}

void p6::type_checker::do_if_node(p6::if_node *const node, int lvl)
{
  node->condition()->accept(this, lvl + 4);
}

void p6::type_checker::do_if_else_node(p6::if_else_node *const node, int lvl)
{
  node->condition()->accept(this, lvl + 4);
}

void p6::type_checker::do_block_node(p6::block_node *const node, int lvl)
{
  if (node->declarations())
    node->declarations()->accept(this, lvl + 2);
  if (node->instructions())
    node->instructions()->accept(this, lvl + 2);
}

void p6::type_checker::do_program_node(p6::program_node *const node, int lvl)
{
  node->block()->accept(this, lvl);
}

//---------------------------------------------------------------------------
// Declarations -- register the symbol in the table.

void p6::type_checker::do_variable_declaration_node(p6::variable_declaration_node *const node, int lvl)
{
  if (node->initializer())
    node->initializer()->accept(this, lvl + 2);
  auto type = node->type();
  if (type == nullptr && node->initializer()) // 'auto': infer from the initializer
    type = node->initializer()->type();
  if (type == nullptr)
    type = int_type();
  _symtab.insert(node->identifier(), std::make_shared<p6::symbol>(type, node->identifier(), 0));
}

void p6::type_checker::do_function_definition_node(p6::function_definition_node *const node, int lvl)
{
  _symtab.insert(node->identifier(),
                 std::make_shared<p6::symbol>(node->type(), node->identifier(), 0));
  if (node->arguments())
    node->arguments()->accept(this, lvl + 2);
  node->block()->accept(this, lvl + 2);
}

void p6::type_checker::do_function_declaration_node(p6::function_declaration_node *const node, int lvl)
{
  _symtab.insert(node->identifier(),
                 std::make_shared<p6::symbol>(node->type(), node->identifier(), 0));
  if (node->arguments())
    node->arguments()->accept(this, lvl + 2);
}