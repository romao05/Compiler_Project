#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include ".auto/all_nodes.h" // all_nodes.h is automatically generated

//---------------------------------------------------------------------------

void p6::postfix_writer::do_nil_node(cdk::nil_node *const node, int lvl)
{
  // EMPTY
}
void p6::postfix_writer::do_data_node(cdk::data_node *const node, int lvl)
{
  // EMPTY
}
void p6::postfix_writer::do_double_node(cdk::double_node *const node, int lvl)
{
  // EMPTY
}
void p6::postfix_writer::do_balanced3_node(cdk::balanced3_node *const node, int lvl)
{
  _pf.BALANCED3(node->value());
}
void p6::postfix_writer::do_posit3_node(cdk::posit3_node *const node, int lvl)
{
  // EMPTY
}
void p6::postfix_writer::do_takum3_node(cdk::takum3_node *const node, int lvl)
{
  _pf.TAKUM3(node->value());
}
void p6::postfix_writer::do_not_node(cdk::not_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  _pf.NOT();
}
void p6::postfix_writer::do_and_node(cdk::and_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  int lbl_false = ++_lbl, lbl_end = ++_lbl;
  node->left()->accept(this, lvl);
  _pf.JZ(mklbl(lbl_false));               // 1º == 0 ? -> falso (curto-circuito)
  node->right()->accept(this, lvl);
  _pf.JZ(mklbl(lbl_false));               // 2º == 0 ? -> falso
  _pf.BALANCED3(cdk::balanced3_type::value_type(1));  // ambos != 0 -> 1
  _pf.JMP(mklbl(lbl_end));
  _pf.LABEL(mklbl(lbl_false));
  _pf.BALANCED3(cdk::balanced3_type::value_type(0));  // -> 0
  _pf.LABEL(mklbl(lbl_end));
}
void p6::postfix_writer::do_or_node(cdk::or_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  int lbl_true = ++_lbl, lbl_end = ++_lbl;
  node -> left() -> accept (this, lvl);
  _pf.JZ(mklbl(lbl_true));
  node -> right() -> accept (this, lvl);
  _pf.JZ(mklbl(lbl_true));
  _pf.BALANCED3(cdk::balanced3_type::value_type(0));
  _pf.JMP(mklbl(lbl_end));
  _pf.LABEL(mklbl(lbl_true));
  _pf.BALANCED3(cdk::balanced3_type::value_type(1));
  _pf.LABEL(mklbl(lbl_end));
}

//---------------------------------------------------------------------------

void p6::postfix_writer::do_sequence_node(cdk::sequence_node *const node, int lvl)
{
  for (size_t i = 0; i < node->size(); i++)
  {
    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void p6::postfix_writer::do_integer_node(cdk::integer_node *const node, int lvl)
{
  _pf.INT(node->value()); // push an integer
}

void p6::postfix_writer::do_string_node(cdk::string_node *const node, int lvl)
{
  int lbl1;

  /* generate the string */
  _pf.RODATA();                    // strings are DATA readonly
  _pf.ALIGN();                     // make sure we are aligned
  _pf.LABEL(mklbl(lbl1 = ++_lbl)); // give the string a name
  _pf.SSTRING(node->value());      // output string characters

  /* leave the address on the stack */
  _pf.TEXT();            // return to the TEXT segment
  _pf.ADDR(mklbl(lbl1)); // the string to be printed
}

//---------------------------------------------------------------------------

void p6::postfix_writer::do_unary_minus_node(cdk::unary_minus_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
  if (node->is_typed(cdk::TYPE_TAKUM3))
    _pf.TNEG();
  else
    _pf.BNEG();
}

void p6::postfix_writer::do_unary_plus_node(cdk::unary_plus_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
}

//---------------------------------------------------------------------------

void p6::postfix_writer::do_add_node(cdk::add_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_TAKUM3) && node->left()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_TAKUM3) && node->right()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  if (node->is_typed(cdk::TYPE_TAKUM3))
    _pf.TADD();
  else
    _pf.BADD();
}

void p6::postfix_writer::do_sub_node(cdk::sub_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_TAKUM3) && node->left()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_TAKUM3) && node->right()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  if (node->is_typed(cdk::TYPE_TAKUM3))
    _pf.TSUB();
  else
    _pf.BSUB();
}
void p6::postfix_writer::do_mul_node(cdk::mul_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_TAKUM3) && node->left()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_TAKUM3) && node->right()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  if (node->is_typed(cdk::TYPE_TAKUM3))
    _pf.TMUL();
  else
    _pf.BMUL();
}
void p6::postfix_writer::do_div_node(cdk::div_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_TAKUM3) && node->left()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_TAKUM3) && node->right()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  if (node->is_typed(cdk::TYPE_TAKUM3))
    _pf.TDIV();
  else
    _pf.BDIV();
}
void p6::postfix_writer::do_mod_node(cdk::mod_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.BMOD(); // Apenas existe mod para inteiros, não para reais ternários
}
void p6::postfix_writer::do_lt_node(cdk::lt_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  bool isTakum3 = node->left()->is_typed(cdk::TYPE_TAKUM3) || node->right()->is_typed(cdk::TYPE_TAKUM3);
  node->left()->accept(this, lvl);
  if (isTakum3 && node->left()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  node->right()->accept(this, lvl);
  if (isTakum3 && node->right()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  _pf.LT();
}
void p6::postfix_writer::do_le_node(cdk::le_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  bool isTakum3 = node->left()->is_typed(cdk::TYPE_TAKUM3) || node->right()->is_typed(cdk::TYPE_TAKUM3);
  node->left()->accept(this, lvl);
  if (isTakum3 && node->left()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  node->right()->accept(this, lvl);
  if (isTakum3 && node->right()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  _pf.LE();
}
void p6::postfix_writer::do_ge_node(cdk::ge_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  bool isTakum3 = node->left()->is_typed(cdk::TYPE_TAKUM3) || node->right()->is_typed(cdk::TYPE_TAKUM3);
  node->left()->accept(this, lvl);
  if (isTakum3 && node->left()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  node->right()->accept(this, lvl);
  if (isTakum3 && node->right()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  _pf.GE();
}
void p6::postfix_writer::do_gt_node(cdk::gt_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  bool isTakum3 = node->left()->is_typed(cdk::TYPE_TAKUM3) || node->right()->is_typed(cdk::TYPE_TAKUM3);
  node->left()->accept(this, lvl);
  if (isTakum3 && node->left()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  node->right()->accept(this, lvl);
  if (isTakum3 && node->right()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  _pf.GT();
}
void p6::postfix_writer::do_ne_node(cdk::ne_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  bool isTakum3 = node->left()->is_typed(cdk::TYPE_TAKUM3) || node->right()->is_typed(cdk::TYPE_TAKUM3);
  node->left()->accept(this, lvl);
  if (isTakum3 && node->left()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  node->right()->accept(this, lvl);
  if (isTakum3 && node->right()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  _pf.NE();
}
void p6::postfix_writer::do_eq_node(cdk::eq_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  bool isTakum3 = node->left()->is_typed(cdk::TYPE_TAKUM3) || node->right()->is_typed(cdk::TYPE_TAKUM3);
  node->left()->accept(this, lvl);
  if (isTakum3 && node->left()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  node->right()->accept(this, lvl);
  if (isTakum3 && node->right()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  _pf.EQ();
}

//---------------------------------------------------------------------------

void p6::postfix_writer::do_variable_node(cdk::variable_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  // simplified generation: all variables are global
  _pf.ADDR(node->name());
}

void p6::postfix_writer::do_rvalue_node(cdk::rvalue_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_TAKUM3))
    _pf.LDTAKUM3();
  else
    _pf.LDBALANCED3();
}

void p6::postfix_writer::do_assignment_node(cdk::assignment_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  bool isTakum3 = node->is_typed(cdk::TYPE_TAKUM3);
  node->rvalue()->accept(this, lvl);
  if (isTakum3)
    _pf.DUP128(); // 128 para 16 bytes
  else
    _pf.DUP64(); // 64 para 8 bytes

  if (new_symbol() == nullptr)
  {
    node->lvalue()->accept(this, lvl); // where to store the value
  }
  else
  {
    _pf.DATA();                      // variables are all global and live in DATA
    _pf.ALIGN();                     // make sure we are aligned
    _pf.LABEL(new_symbol()->name()); // name variable location
    reset_new_symbol();
    if (isTakum3)
      _pf.STAKUM3(cdk::takum3_type::value_type(0));
    else
      _pf.SBALANCED3(cdk::balanced3_type::value_type(0)); // initialize it to 0 (zero)
    _pf.TEXT();                                           // return to the TEXT segment
    node->lvalue()->accept(this, lvl);                    // DAVID: bah!
  }
  if (isTakum3)
    _pf.STTAKUM3();
  else
    _pf.STBALANCED3();
}

//---------------------------------------------------------------------------

void p6::postfix_writer::do_program_node(p6::program_node *const node, int lvl)
{
  // Note that Simple doesn't have functions. Thus, it doesn't need
  // a function node. However, it must start in the main function.
  // The ProgramNode (representing the whole program) doubles as a
  // main function node.

  // generate the main function (RTS mandates that its name be "_main")
  _pf.TEXT();
  _pf.ALIGN();
  _pf.GLOBAL("_main", _pf.FUNC());
  _pf.LABEL("_main");
  _pf.ENTER(0); // Simple doesn't implement local variables

  node->block()->accept(this, lvl);

  // end the main function
  _pf.INT(0);
  _pf.STFVAL32I();
  _pf.LEAVE();
  _pf.RET();

  // these are just a few library function imports
  _pf.EXTERN("readi");
  _pf.EXTERN("printi");
  _pf.EXTERN("prints");
  _pf.EXTERN("println");

  // RTS helpers called explicitly via _pf.CALL (native opcodes like BADD
  // auto-declare their own helpers; these manual CALLs do not, so import them)
  _pf.EXTERN("balanced3_print");
  _pf.EXTERN("balanced3_read");
  _pf.EXTERN("takum3_print");
  _pf.EXTERN("takum3_read");
}

//---------------------------------------------------------------------------

void p6::postfix_writer::do_evaluation_node(p6::evaluation_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
  if (node->argument()->is_typed(cdk::TYPE_TAKUM3))
  {
    _pf.TRASH(16); // delete the evaluated value
  }
  else if (node->argument()->is_typed(cdk::TYPE_BALANCED3))
  {
    _pf.TRASH(8); // delete the evaluated value's address
  }
  else if (node->argument()->is_typed(cdk::TYPE_STRING)) 
  {
    _pf.TRASH(4); // delete the evaluated value's address
  }
  else
  {
    std::cerr << "ERROR: CANNOT HAPPEN!" << std::endl;
    exit(1);
  }
}

void p6::postfix_writer::do_write_node(p6::write_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  for (size_t i = 0; i < node->arguments()->size(); i++)
  {
    auto arg = dynamic_cast<cdk::expression_node *>(node->arguments()->node(i));
    arg->accept(this, lvl); // determine the value to print
    if (arg->is_typed(cdk::TYPE_BALANCED3))
    {
      _pf.CALL("balanced3_print");
      _pf.TRASH(8); // delete the printed balanced3 value
    }
    else if (arg->is_typed(cdk::TYPE_TAKUM3))
    {
      _pf.CALL("takum3_print");
      _pf.TRASH(16); // delete the printed takum3 value
    }
    else if (arg->is_typed(cdk::TYPE_STRING))
    {
      _pf.CALL("prints");
      _pf.TRASH(4); // delete the printed value's address
    }
    else
    {
      std::cerr << "ERROR: CANNOT HAPPEN!" << std::endl;
      exit(1);
    }
    _pf.CALL("println"); // print a newline
  }
  if (node->newline())
    _pf.CALL("println");
}

//---------------------------------------------------------------------------

void p6::postfix_writer::do_input_node(p6::input_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  if (node->is_typed(cdk::TYPE_BALANCED3))
  { // int: 64 bits, devolvido em eax:edx
    _pf.CALL("balanced3_read");
    _pf.LDFVAL64I();
  }
  else
  { // real (takum3): 128 bits, sret por ponteiro
    _pf.INT(16);
    _pf.ALLOC();
    _pf.SP();
    _pf.CALL("takum3_read");
    _pf.TRASH(4);
  }
}

//---------------------------------------------------------------------------

void p6::postfix_writer::do_block_node(p6::block_node *const node, int lvl)
{
  _symtab.push(); // for block-local vars
  if (node->declarations())
    node->declarations()->accept(this, lvl + 2);
  if (node->instructions())
    node->instructions()->accept(this, lvl + 2);
  _symtab.pop();
}

void p6::postfix_writer::do_variable_declaration_node(p6::variable_declaration_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  bool isTakum3 = node->is_typed(cdk::TYPE_TAKUM3);
  if (node->qualifier() == QUALIFIER_EXTERN || node->qualifier() == QUALIFIER_FORWARD) {
    _pf.EXTERN(node->identifier());
    return;
  }
  else if (node->qualifier() == QUALIFIER_PUBLIC) {
    _pf.GLOBAL(node->identifier(), _pf.OBJ());
  }

  _pf.DATA();
  _pf.ALIGN();
  _pf.LABEL(node->identifier());
  if (isTakum3)
    _pf.STAKUM3(cdk::takum3_type::value_type(0));
  else
    _pf.SBALANCED3(cdk::balanced3_type::value_type(0));
  _pf.TEXT();

  if (node->initializer() != nullptr) {
    node->initializer()->accept(this, lvl);
    _pf.ADDR(node->identifier());
    if (isTakum3)
      _pf.STTAKUM3();
    else
      _pf.STBALANCED3();
  }
}

void p6::postfix_writer::do_function_definition_node(p6::function_definition_node *const node, int lvl)
{
  // EMPTY
}

void p6::postfix_writer::do_function_declaration_node(p6::function_declaration_node *const node, int lvl)
{
  // EMPTY
}

void p6::postfix_writer::do_function_call_node(p6::function_call_node *const node, int lvl)
{
  // EMPTY
}

void p6::postfix_writer::do_null_node(p6::null_node *const node, int lvl)
{
  // EMPTY
}

void p6::postfix_writer::do_sizeof_node(p6::sizeof_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  _pf.INT(node->expression()->type()->size());
}

void p6::postfix_writer::do_address_of_node(p6::address_of_node *const node, int lvl)
{
  // EMPTY
}

void p6::postfix_writer::do_index_node(p6::index_node *const node, int lvl)
{
  // EMPTY
}

void p6::postfix_writer::do_stack_alloc_node(p6::stack_alloc_node *const node, int lvl)
{
  // EMPTY
}

void p6::postfix_writer::do_return_node(p6::return_node *const node, int lvl)
{
  // EMPTY
}

void p6::postfix_writer::do_stop_node(p6::stop_node *const node, int lvl)
{
  // EMPTY
}

void p6::postfix_writer::do_next_node(p6::next_node *const node, int lvl)
{
  // EMPTY
}

//---------------------------------------------------------------------------

void p6::postfix_writer::do_while_node(p6::while_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  _pf.LABEL(mklbl(lbl1 = ++_lbl));
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl2 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl1));
  _pf.LABEL(mklbl(lbl2));
}

//---------------------------------------------------------------------------

void p6::postfix_writer::do_if_node(p6::if_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1));
}

//---------------------------------------------------------------------------

void p6::postfix_writer::do_if_else_node(p6::if_else_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->thenblock()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl2 = ++_lbl));
  _pf.LABEL(mklbl(lbl1));
  node->elseblock()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1 = lbl2));
}
