#include <string>
#include <sstream>
#include <cdk/types/reference_type.h>
#include <cdk/types/functional_type.h>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include "targets/frame_size_calculator.h"
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
  if (_inFunctionBody)
    _pf.BALANCED3(node->value()); // stack (TEXT)
  else
    _pf.SBALANCED3(node->value()); // DATA segment
}
void p6::postfix_writer::do_posit3_node(cdk::posit3_node *const node, int lvl)
{
  // EMPTY
}
void p6::postfix_writer::do_takum3_node(cdk::takum3_node *const node, int lvl)
{
  if (_inFunctionBody)
    _pf.TAKUM3(node->value()); // stack (TEXT)
  else
    _pf.STAKUM3(node->value()); // DATA segment
}
void p6::postfix_writer::do_not_node(cdk::not_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  // Lógica de Kleene: negação = -sinal(x)  (T/+ -> F/-1, U/0 -> U/0, F/- -> T/+1)
  node->argument()->accept(this, lvl);
  _pf.KNOT();
}
void p6::postfix_writer::do_and_node(cdk::and_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  // Kleene "e" = min(sinal(esq), sinal(dir)).
  // Curto-circuito: o 2º só é avaliado se o 1º NÃO for falso (negativo).
  int lbl_false = ++_lbl, lbl_end = ++_lbl;
  node->left()->accept(this, lvl);  // [L]          (balanced3, 8 bytes)
  _pf.DUP64();                      // [L, L]       (preserva L para o KAND)
  _pf.B2I();                        // [L, iL]      iL = valor inteiro de L (4 bytes)
  _pf.INT(0);                       // [L, iL, 0]   (inteiro de 4 bytes)
  _pf.LT();                         // [L, c]       c = (iL < 0) ? 1 : 0
  _pf.JNZ(mklbl(lbl_false));        // esq falso -> curto-circuito (pop 4 bytes)
  node->right()->accept(this, lvl); // [L, R]
  _pf.KAND();                       // [min(sinal L, sinal R)]
  _pf.JMP(mklbl(lbl_end));
  _pf.LABEL(mklbl(lbl_false));
  _pf.TRASH(8);                                       // descarta o L preservado
  _pf.BALANCED3(cdk::balanced3_type::value_type(-1)); // resultado -> FALSO
  _pf.LABEL(mklbl(lbl_end));
}
void p6::postfix_writer::do_or_node(cdk::or_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  // Kleene "ou" = max(sinal(esq), sinal(dir)).
  // Curto-circuito: o 2º só é avaliado se o 1º NÃO for verdadeiro (positivo).
  int lbl_true = ++_lbl, lbl_end = ++_lbl;
  node->left()->accept(this, lvl);  // [L]          (balanced3, 8 bytes)
  _pf.DUP64();                      // [L, L]       (preserva L para o KOR)
  _pf.B2I();                        // [L, iL]      iL = valor inteiro de L (4 bytes)
  _pf.INT(0);                       // [L, iL, 0]   (inteiro de 4 bytes)
  _pf.GT();                         // [L, c]       c = (iL > 0) ? 1 : 0
  _pf.JNZ(mklbl(lbl_true));         // esq verdadeiro -> curto-circuito (pop 4 bytes)
  node->right()->accept(this, lvl); // [L, R]
  _pf.KOR();                        // [max(sinal L, sinal R)]
  _pf.JMP(mklbl(lbl_end));
  _pf.LABEL(mklbl(lbl_true));
  _pf.TRASH(8);                                      // descarta o L preservado
  _pf.BALANCED3(cdk::balanced3_type::value_type(1)); // resultado -> VERDADEIRO
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
  if (_inFunctionBody)
    _pf.INT(node->value()); // stack (TEXT)
  else
    _pf.SINT(node->value()); // DATA segment
}

void p6::postfix_writer::do_string_node(cdk::string_node *const node, int lvl)
{
  int lbl1;

  _pf.RODATA();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1 = ++_lbl));
  _pf.SSTRING(node->value());

  if (_inFunctionBody)
  {
    _pf.TEXT();
    _pf.ADDR(mklbl(lbl1)); // address onto the stack
  }
  else
  {
    _pf.DATA();
    _pf.SADDR(mklbl(lbl1)); // address into DATA segment
  }
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
  // Pointer + integer: the result is a pointer (4-byte binary address). The
  // integer index must be converted to 32-bit binary (B2I) and scaled by the
  // size of the pointed-to element before being added to the address. This
  // mirrors do_index_node's offset computation.
  if (node->is_typed(cdk::TYPE_POINTER))
  {
    auto ref = cdk::reference_type::cast(node->type());
    int elem = (ref && ref->referenced()) ? ref->referenced()->size() : 1;
    bool leftPtr = node->left()->is_typed(cdk::TYPE_POINTER);
    auto ptr = leftPtr ? node->left() : node->right();
    auto idx = leftPtr ? node->right() : node->left();
    ptr->accept(this, lvl); // base address (4 bytes)
    idx->accept(this, lvl); // index (balanced3, 8 bytes)
    _pf.B2I();              // -> binary 32-bit index
    _pf.INT(elem);
    _pf.MUL(); // index * element size (32-bit)
    _pf.ADD(); // base + offset
    return;
  }
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
  // Pointer - pointer: result is the (integer) element distance between the two
  // addresses: (a - b) / element_size, converted back to balanced3. Checked
  // before the pointer-integer case because the type checker types both as a
  // pointer.
  if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_POINTER))
  {
    auto ref = cdk::reference_type::cast(node->left()->type());
    int elem = (ref && ref->referenced()) ? ref->referenced()->size() : 1;
    node->left()->accept(this, lvl);
    node->right()->accept(this, lvl);
    _pf.SUB(); // byte distance (32-bit)
    _pf.INT(elem);
    _pf.DIV(); // element distance (32-bit)
    _pf.I2B(); // -> balanced3 (int)
    return;
  }
  // Pointer - integer: result is a pointer; scale the index by the element size
  // and subtract (32-bit binary arithmetic, like do_add_node).
  if (node->is_typed(cdk::TYPE_POINTER))
  {
    auto ref = cdk::reference_type::cast(node->type());
    int elem = (ref && ref->referenced()) ? ref->referenced()->size() : 1;
    node->left()->accept(this, lvl);  // base address (4 bytes)
    node->right()->accept(this, lvl); // index (balanced3, 8 bytes)
    _pf.B2I();                        // -> binary 32-bit index
    _pf.INT(elem);
    _pf.MUL(); // index * element size (32-bit)
    _pf.SUB(); // base - offset
    return;
  }
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
  if (node->left()->is_typed(cdk::TYPE_TAKUM3) || node->right()->is_typed(cdk::TYPE_TAKUM3))
  {
    node->right()->accept(this, lvl);
    if (node->right()->is_typed(cdk::TYPE_BALANCED3))
      _pf.B2T();
    node->left()->accept(this, lvl);
    if (node->left()->is_typed(cdk::TYPE_BALANCED3))
      _pf.B2T();
    _pf.CALL("takum3_lt");
    _pf.TRASH(32);
    _pf.LDFVAL64I(); // a RTS devolve o booleano P6 já como balanced3 (-1/+1)
    return;          // não aplicar 2*x-1/I2B (conversão só para o caso inteiro)
  }
  else
  {
    node->left()->accept(this, lvl);
    if (node->left()->is_typed(cdk::TYPE_BALANCED3))
    {
      _pf.B2I();
    }
    node->right()->accept(this, lvl);
    if (node->right()->is_typed(cdk::TYPE_BALANCED3))
    {
      _pf.B2I();
    }
    _pf.LT();
  }
  // P6 booleano (ternário): falso = -1, verdadeiro = +1.
  // Converte o 0/1 do ALU inteiro em -1/+1 com 2*x-1.
  _pf.INT(2);
  _pf.MUL();
  _pf.INT(1);
  _pf.SUB();
  _pf.I2B(); // -1/+1 (4 bytes) -> balanced3 (8 bytes), conforme o tipo estático
}
void p6::postfix_writer::do_le_node(cdk::le_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  if (node->left()->is_typed(cdk::TYPE_TAKUM3) || node->right()->is_typed(cdk::TYPE_TAKUM3))
  {
    node->right()->accept(this, lvl);
    if (node->right()->is_typed(cdk::TYPE_BALANCED3))
      _pf.B2T();
    node->left()->accept(this, lvl);
    if (node->left()->is_typed(cdk::TYPE_BALANCED3))
      _pf.B2T();
    _pf.CALL("takum3_le");
    _pf.TRASH(32);
    _pf.LDFVAL64I(); // a RTS devolve o booleano P6 já como balanced3 (-1/+1)
    return;          // não aplicar 2*x-1/I2B (conversão só para o caso inteiro)
  }
  else
  {
    node->left()->accept(this, lvl);
    if (node->left()->is_typed(cdk::TYPE_BALANCED3))
    {
      _pf.B2I();
    }
    node->right()->accept(this, lvl);
    if (node->right()->is_typed(cdk::TYPE_BALANCED3))
    {
      _pf.B2I();
    }
    _pf.LE();
  }
  // P6 booleano (ternário): falso = -1, verdadeiro = +1.
  // Converte o 0/1 do ALU inteiro em -1/+1 com 2*x-1.
  _pf.INT(2);
  _pf.MUL();
  _pf.INT(1);
  _pf.SUB();
  _pf.I2B(); // -1/+1 (4 bytes) -> balanced3 (8 bytes), conforme o tipo estático
}
void p6::postfix_writer::do_ge_node(cdk::ge_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  if (node->left()->is_typed(cdk::TYPE_TAKUM3) || node->right()->is_typed(cdk::TYPE_TAKUM3))
  {
    node->right()->accept(this, lvl);
    if (node->right()->is_typed(cdk::TYPE_BALANCED3))
      _pf.B2T();
    node->left()->accept(this, lvl);
    if (node->left()->is_typed(cdk::TYPE_BALANCED3))
      _pf.B2T();
    _pf.CALL("takum3_ge");
    _pf.TRASH(32);
    _pf.LDFVAL64I(); // a RTS devolve o booleano P6 já como balanced3 (-1/+1)
    return;          // não aplicar 2*x-1/I2B (conversão só para o caso inteiro)
  }
  else
  {
    node->left()->accept(this, lvl);
    if (node->left()->is_typed(cdk::TYPE_BALANCED3))
    {
      _pf.B2I();
    }
    node->right()->accept(this, lvl);
    if (node->right()->is_typed(cdk::TYPE_BALANCED3))
    {
      _pf.B2I();
    }
    _pf.GE();
  }
  // P6 booleano (ternário): falso = -1, verdadeiro = +1.
  // Converte o 0/1 do ALU inteiro em -1/+1 com 2*x-1.
  _pf.INT(2);
  _pf.MUL();
  _pf.INT(1);
  _pf.SUB();
  _pf.I2B(); // -1/+1 (4 bytes) -> balanced3 (8 bytes), conforme o tipo estático
}
void p6::postfix_writer::do_gt_node(cdk::gt_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  if (node->left()->is_typed(cdk::TYPE_TAKUM3) || node->right()->is_typed(cdk::TYPE_TAKUM3))
  {
    node->right()->accept(this, lvl);
    if (node->right()->is_typed(cdk::TYPE_BALANCED3))
      _pf.B2T();
    node->left()->accept(this, lvl);
    if (node->left()->is_typed(cdk::TYPE_BALANCED3))
      _pf.B2T();
    _pf.CALL("takum3_gt");
    _pf.TRASH(32);
    _pf.LDFVAL64I(); // a RTS devolve o booleano P6 já como balanced3 (-1/+1)
    return;          // não aplicar 2*x-1/I2B (conversão só para o caso inteiro)
  }
  else
  {
    node->left()->accept(this, lvl);
    if (node->left()->is_typed(cdk::TYPE_BALANCED3))
    {
      _pf.B2I();
    }
    node->right()->accept(this, lvl);
    if (node->right()->is_typed(cdk::TYPE_BALANCED3))
    {
      _pf.B2I();
    }
    _pf.GT();
  }
  // P6 booleano (ternário): falso = -1, verdadeiro = +1.
  // Converte o 0/1 do ALU inteiro em -1/+1 com 2*x-1.
  _pf.INT(2);
  _pf.MUL();
  _pf.INT(1);
  _pf.SUB();
  _pf.I2B(); // -1/+1 (4 bytes) -> balanced3 (8 bytes), conforme o tipo estático
}
void p6::postfix_writer::do_ne_node(cdk::ne_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  if (node->left()->is_typed(cdk::TYPE_TAKUM3) || node->right()->is_typed(cdk::TYPE_TAKUM3))
  {
    node->right()->accept(this, lvl);
    if (node->right()->is_typed(cdk::TYPE_BALANCED3))
      _pf.B2T();
    node->left()->accept(this, lvl);
    if (node->left()->is_typed(cdk::TYPE_BALANCED3))
      _pf.B2T();
    _pf.CALL("takum3_ne");
    _pf.TRASH(32);
    _pf.LDFVAL64I(); // a RTS devolve o booleano P6 já como balanced3 (-1/+1)
    return;          // não aplicar 2*x-1/I2B (conversão só para o caso inteiro)
  }
  else
  {
    node->left()->accept(this, lvl);
    if (node->left()->is_typed(cdk::TYPE_BALANCED3))
    {
      _pf.B2I();
    }
    node->right()->accept(this, lvl);
    if (node->right()->is_typed(cdk::TYPE_BALANCED3))
    {
      _pf.B2I();
    }
    _pf.NE();
  }
  // P6 booleano (ternário): falso = -1, verdadeiro = +1.
  // Converte o 0/1 do ALU inteiro em -1/+1 com 2*x-1.
  _pf.INT(2);
  _pf.MUL();
  _pf.INT(1);
  _pf.SUB();
  _pf.I2B(); // -1/+1 (4 bytes) -> balanced3 (8 bytes), conforme o tipo estático
}
void p6::postfix_writer::do_eq_node(cdk::eq_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  if (node->left()->is_typed(cdk::TYPE_TAKUM3) || node->right()->is_typed(cdk::TYPE_TAKUM3))
  {
    // comparação de reais: feita pela RTS em precisão ternária completa.
    // Cdecl quer arg0 (esq) no topo, logo a dir é empilhada primeiro; operando
    // inteiro é promovido a real com B2T.
    node->right()->accept(this, lvl);
    if (node->right()->is_typed(cdk::TYPE_BALANCED3))
      _pf.B2T();
    node->left()->accept(this, lvl);
    if (node->left()->is_typed(cdk::TYPE_BALANCED3))
      _pf.B2T();
    _pf.CALL("takum3_eq");
    _pf.TRASH(32);   // dois operandos takum3 (16 bytes cada)
    _pf.LDFVAL64I(); // a RTS devolve o booleano P6 já como balanced3 (-1/+1)
    return;          // não aplicar 2*x-1/I2B (conversão só para o caso inteiro)
  }
  else
  {
    node->left()->accept(this, lvl);
    if (node->left()->is_typed(cdk::TYPE_BALANCED3))
    {
      _pf.B2I();
    }
    node->right()->accept(this, lvl);
    if (node->right()->is_typed(cdk::TYPE_BALANCED3))
    {
      _pf.B2I();
    }
    _pf.EQ();
  }
  // P6 booleano (ternário): falso = -1, verdadeiro = +1.
  // Converte o 0/1 do ALU inteiro em -1/+1 com 2*x-1.
  _pf.INT(2);
  _pf.MUL();
  _pf.INT(1);
  _pf.SUB();
  _pf.I2B(); // -1/+1 (4 bytes) -> balanced3 (8 bytes), conforme o tipo estático
}

//---------------------------------------------------------------------------

void p6::postfix_writer::do_variable_node(cdk::variable_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  auto sym = _symtab.find(node->name());
  if (sym && sym->value() != 0)
    _pf.LOCAL(sym->value());
  else
    _pf.ADDR(node->name());
}

void p6::postfix_writer::do_rvalue_node(cdk::rvalue_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_TAKUM3))
    _pf.LDTAKUM3(); // real: 16 bytes
  else if (node->is_typed(cdk::TYPE_BALANCED3))
    _pf.LDBALANCED3(); // int: 8 bytes
  else
    _pf.LDINT(); // string or pointer: 4 bytes
}

void p6::postfix_writer::do_assignment_node(cdk::assignment_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  bool isTakum3 = node->is_typed(cdk::TYPE_TAKUM3);
  bool isBalanced3 = node->is_typed(cdk::TYPE_BALANCED3);
  node->rvalue()->accept(this, lvl);
  // implicit int -> real conversion (e.g. `r = 1;` with real r)
  if (isTakum3 && node->rvalue()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2T();
  if (isTakum3)
    _pf.DUP128(); // real: 16 bytes
  else if (isBalanced3)
    _pf.DUP64(); // int: 8 bytes
  else
    _pf.DUP32(); // string or pointer: 4 bytes

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
    else if (isBalanced3)
      _pf.SBALANCED3(cdk::balanced3_type::value_type(0)); // initialize it to 0 (zero)
    else
      _pf.SALLOC(4);   // string or pointer (4 bytes)
    _pf.TEXT();                        // return to the TEXT segment
    node->lvalue()->accept(this, lvl); // DAVID: bah!
  }
  if (isTakum3)
    _pf.STTAKUM3();
  else if (isBalanced3)
    _pf.STBALANCED3();
  else
    _pf.STINT(); // string or pointer: 4 bytes
}

//---------------------------------------------------------------------------

void p6::postfix_writer::do_program_node(p6::program_node *const node, int lvl)
{
  // Type-check the whole block first so that every declaration (notably
  // 'auto' variables, whose type is only known after inference) already has a
  // type before the frame_size_calculator reads node->type()->size().
  // Without this the calculator dereferences a null type and segfaults.
  // A semantic error here is swallowed on purpose: it is reported (and code
  // generation aborted at the right place) by the per-node checks during the
  // actual walk below, just as it was before this pre-pass existed.
  _symtab.push();
  try
  {
    p6::type_checker checker(_compiler, _symtab, this);
    node->block()->accept(&checker, 0);
  }
  catch (const std::string &)
  { /* reported later, during code generation */
  }
  _symtab.pop();
  reset_new_symbol();

  frame_size_calculator fsc(_compiler);
  node->block()->accept(&fsc, lvl);

  _pf.TEXT();
  _pf.ALIGN();
  _pf.GLOBAL("_main", _pf.FUNC());
  _pf.LABEL("_main");
  _pf.ENTER(fsc.localsize());

  // main behaves like an int-returning function for the sake of "return"
  _function = std::make_shared<p6::symbol>(cdk::primitive_type::create(8, cdk::TYPE_BALANCED3), "_main", 0);
  _funcEndLabel = ++_lbl;
  _offset = 0;
  _inFunctionBody = true;
  node->block()->accept(this, lvl);
  _inFunctionBody = false;

  // A p6 program always exits 0 on success. The user's "return 0" stores the
  // *balanced3 encoding* of 0 into eax (which is non-zero), so the OS would see
  // a bogus exit code. Force a native 0 here, after the end label, so it runs
  // both on fall-through and on the "return" path.
  _pf.LABEL(mklbl(_funcEndLabel));
  _pf.INT(0);
  _pf.STFVAL32I(); // native 0 -> exit code 0
  _pf.LEAVE();
  _pf.RET();

  // 'forward'-declared functions that are never defined in this module are
  // genuinely external: emit their 'extern' now (those defined locally are not).
  for (const auto &name : _forwardFunctions)
    if (_definedFunctions.find(name) == _definedFunctions.end())
      _pf.EXTERN(name);

  _pf.EXTERN("readi");
  _pf.EXTERN("printi");
  _pf.EXTERN("prints");
  _pf.EXTERN("println");
  _pf.EXTERN("balanced3_print");
  _pf.EXTERN("balanced3_read");
  _pf.EXTERN("takum3_print");
  _pf.EXTERN("takum3_read");
  _pf.EXTERN("takum3_eq");
  _pf.EXTERN("takum3_ne");
  _pf.EXTERN("takum3_lt");
  _pf.EXTERN("takum3_le");
  _pf.EXTERN("takum3_gt");
  _pf.EXTERN("takum3_ge");
}

//---------------------------------------------------------------------------

void p6::postfix_writer::do_evaluation_node(p6::evaluation_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
  // A void expression (e.g. a call to a void function used as a statement)
  // leaves nothing on the stack; everything else must have its result discarded.
  if (!node->argument()->is_typed(cdk::TYPE_VOID))
    _pf.TRASH(node->argument()->type()->size());
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

  if (node->qualifier() == QUALIFIER_EXTERN || node->qualifier() == QUALIFIER_FORWARD)
  {
    _pf.EXTERN(node->identifier());
    reset_new_symbol();
    return;
  }

  if (_inFunctionArgs)
  {
    // The arguments are walked here BEFORE the body pre-pass type-checker runs,
    // so new_symbol() has not been set for them. Create and insert the symbol
    // ourselves so the parameter is visible (with its frame offset) both to the
    // body type-check and to code generation. Without this, references to the
    // parameter throw "undeclared" and abort the whole compilation.
    auto sym = new_symbol();
    if (!sym)
    {
      sym = std::make_shared<p6::symbol>(node->type(), node->identifier(), 0);
      _symtab.insert(node->identifier(), sym);
    }
    sym->value(_offset);
    reset_new_symbol();
    _offset += node->type()->size(); // args crescem para cima (offsets positivos)
    return;
  }

  if (_inFunctionBody)
  {
    _offset -= node->type()->size();
    auto sym = new_symbol();
    if (sym)
    {
      sym->value(_offset);
      reset_new_symbol();
    }
    if (node->initializer() != nullptr)
    {
      node->initializer()->accept(this, lvl);
      // implicit int -> real conversion (e.g. `real r = 1;`)
      if (isTakum3 && node->initializer()->is_typed(cdk::TYPE_BALANCED3))
        _pf.B2T();
      _pf.LOCAL(_offset);
      if (isTakum3)
        _pf.STTAKUM3();
      else if (node->is_typed(cdk::TYPE_BALANCED3))
        _pf.STBALANCED3();
      else
        _pf.STINT(); // string or pointer (4 bytes)
    }
    return;
  }

  // global variable: storage is allocated here, so the symbol must not leak
  // into the next assignment (which would emit a duplicate DATA label).
  reset_new_symbol();
  if (node->qualifier() == QUALIFIER_PUBLIC)
    _pf.GLOBAL(node->identifier(), _pf.OBJ());

  _pf.DATA();
  _pf.ALIGN();
  _pf.LABEL(node->identifier());
  if (node->initializer() != nullptr)
  {
    // global initializers are compile-time constants: emit the literal value
    // STRAIGHT into the DATA segment. Because _inFunctionBody is false here,
    // the literal nodes (balanced3/takum3/string) call their S* variants and
    // write the constant directly under this label -- no runtime store needed.
    node->initializer()->accept(this, lvl);
  }
  else if (isTakum3)
    _pf.STAKUM3(cdk::takum3_type::value_type(0));
  else if (node->is_typed(cdk::TYPE_BALANCED3))
    _pf.SBALANCED3(cdk::balanced3_type::value_type(0));
  else
    _pf.SALLOC(4); // string or pointer (4 bytes)
  _pf.TEXT();
}

void p6::postfix_writer::do_function_definition_node(p6::function_definition_node *const node, int lvl)
{

  // remember the enclosing function context (functions may be nested)
  auto previous_function = _function;
  int previous_end = _funcEndLabel;
  int previous_sret = _funcSretOffset;

  _function = std::make_shared<p6::symbol>(node->type(), node->identifier(), 0);
  _symtab.insert(node->identifier(), _function);
  _definedFunctions.insert(node->identifier()); // defined here: no 'extern' needed
  reset_new_symbol();

  // record the formal parameter types so calls can apply implicit int->real
  // conversions to actual arguments (see do_function_call_node)
  {
    std::vector<std::shared_ptr<cdk::basic_type>> argtypes;
    if (node->arguments())
      for (size_t i = 0; i < node->arguments()->size(); i++)
        argtypes.push_back(node->argument(i)->type());
    _funcArgTypes[node->identifier()] = argtypes;
  }

  // process argument declarations (positive offsets from FP)
  _offset = 8; // skip saved FP (4) + return address (4)
  _symtab.push();
  _inFunctionArgs = true;
  if (node->arguments())
    node->arguments()->accept(this, lvl);
  _inFunctionArgs = false;

  // the hidden takum3-return pointer (when present) is pushed before the
  // arguments, so it sits just above them in the frame
  _funcSretOffset = _offset;

  // type-check the body first (see do_program_node) so 'auto' locals have a
  // type before the frame size is computed; a semantic error is swallowed
  // here and reported later by the per-node checks during the walk
  try
  {
    p6::type_checker checker(_compiler, _symtab, this);
    node->block()->accept(&checker, 0);
  }
  catch (const std::string &)
  { /* reported later, during code generation */
  }
  reset_new_symbol();

  // compute local frame size before emitting ENTER
  frame_size_calculator fsc(_compiler);
  node->block()->accept(&fsc, lvl);

  _pf.TEXT();
  _pf.ALIGN();
  if (node->qualifier() == QUALIFIER_PUBLIC)
    _pf.GLOBAL(node->identifier(), _pf.FUNC());
  _pf.LABEL(node->identifier());
  _pf.ENTER(fsc.localsize());

  _funcEndLabel = ++_lbl;
  _offset = 0;
  _inFunctionBody = true;
  node->block()->accept(this, lvl);
  _inFunctionBody = false;

  _pf.LABEL(mklbl(_funcEndLabel)); // return statements jump here
  _pf.LEAVE();
  _pf.RET();
  _symtab.pop();

  // restore the enclosing function context
  _function = previous_function;
  _funcEndLabel = previous_end;
  _funcSretOffset = previous_sret;
}

void p6::postfix_writer::do_function_declaration_node(p6::function_declaration_node *const node, int lvl)
{
  // Register the symbol so calls to this function elsewhere in the module are
  // type-checked correctly (its type is functional; the return type is read off
  // it in do_function_call_node).
  if (_symtab.find(node->identifier()) == nullptr)
    _symtab.insert(node->identifier(),
                   std::make_shared<p6::symbol>(node->type(), node->identifier(), 0));

  // record the formal parameter types (from the functional type) so calls can
  // apply implicit int->real conversions to actual arguments
  {
    std::vector<std::shared_ptr<cdk::basic_type>> argtypes;
    auto ft = cdk::functional_type::cast(node->type());
    if (ft)
      for (size_t i = 0; i < ft->input_length(); i++)
        argtypes.push_back(ft->input(i));
    _funcArgTypes[node->identifier()] = argtypes;
  }

  if (node->qualifier() == QUALIFIER_EXTERN)
  {
    // non-P6 (e.g. C) function: always external
    _pf.EXTERN(node->identifier());
  }
  else
  {
    // 'forward': the function may be defined later in THIS module (mutual
    // recursion) or in another one. Emitting 'extern' now would clash with a
    // local definition, so defer the decision to the end of the module
    // (see do_program_node): emit 'extern' only if it is never defined here.
    _forwardFunctions.insert(node->identifier());
  }
}

void p6::postfix_writer::do_function_call_node(p6::function_call_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;

  bool sret = node->is_typed(cdk::TYPE_TAKUM3);

  // sret: allocate 16 bytes for result; SP() captures the address before args are pushed
  if (sret)
  {
    _pf.INT(16);
    _pf.ALLOC();
    _pf.SP();
  }

  // push arguments right-to-left (Cdecl)
  auto itypes = _funcArgTypes.find(node->identifier());
  int args_size = 0;
  for (int i = (int)node->arguments()->size() - 1; i >= 0; i--)
  {
    auto arg = node->argument(i);
    arg->accept(this, lvl);
    // implicit int -> real conversion when the formal parameter is real
    bool promote = itypes != _funcArgTypes.end() && (size_t)i < itypes->second.size() &&
                   itypes->second[i] && itypes->second[i]->name() == cdk::TYPE_TAKUM3 &&
                   arg->is_typed(cdk::TYPE_BALANCED3);
    if (promote)
    {
      _pf.B2T();
      args_size += 16; // pushed as a real (takum3)
    }
    else
    {
      args_size += arg->type()->size();
    }
  }

  _pf.CALL(node->identifier());
  _pf.TRASH((sret ? 4 : 0) + args_size);

  if (node->is_typed(cdk::TYPE_BALANCED3))
    _pf.LDFVAL64I();
  else if (node->is_typed(cdk::TYPE_STRING) || node->is_typed(cdk::TYPE_POINTER))
    _pf.LDFVAL32I();
  // takum3: result already on stack from ALLOC; void: nothing to load
}

void p6::postfix_writer::do_null_node(p6::null_node *const node, int lvl)
{
  // the null pointer is a 4-byte address with value 0
  if (_inFunctionBody)
    _pf.INT(0);
  else
    _pf.SINT(0);
}

void p6::postfix_writer::do_sizeof_node(p6::sizeof_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  // sizeof is a balanced3 (int): encode the byte size as a balanced ternary value
  cdk::balanced3_type::value_type size(static_cast<long long>(node->expression()->type()->size()));
  if (_inFunctionBody)
    _pf.BALANCED3(size);
  else
    _pf.SBALANCED3(size);
}

void p6::postfix_writer::do_address_of_node(p6::address_of_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
}

void p6::postfix_writer::do_index_node(p6::index_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  // Pointer indexing yields a left-value: leave the *address* of the element on
  // the stack. Pointer arithmetic is binary 32-bit (manual), so the ternary
  // index must be converted with B2I before scaling by the element size.
  node->base()->accept(this, lvl);    // the pointer value (4 bytes)
  node->index()->accept(this, lvl);   // the index (balanced3, 8 bytes)
  _pf.B2I();                          // -> binary 32-bit index
  _pf.INT(node->type()->size());      // size of the pointed-to element
  _pf.MUL();                          // index * element size (32-bit)
  _pf.ADD();                          // base + offset
}

void p6::postfix_writer::do_stack_alloc_node(p6::stack_alloc_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  // Reserve space on the current frame for 'count' objects and return a pointer
  // to it. The element size comes from the (context-inferred) referenced type.
  auto ref = cdk::reference_type::cast(node->type());
  int elem_size = (ref && ref->referenced()) ? ref->referenced()->size() : 1;
  node->argument()->accept(this, lvl); // count (balanced3, 8 bytes)
  _pf.B2I();                           // -> binary 32-bit count
  _pf.INT(elem_size);
  _pf.MUL();                           // total bytes (32-bit)
  _pf.ALLOC();                         // reserve on the stack
  _pf.SP();                            // push the pointer to the reserved area
}

void p6::postfix_writer::do_return_node(p6::return_node *const node, int lvl)
{
  // a non-void function must produce its value before leaving
  if (_function->type()->name() != cdk::TYPE_VOID && node->expression() != nullptr)
  {
    node->expression()->accept(this, lvl + 2);
    // implicit int -> real conversion on the returned value
    if (_function->is_typed(cdk::TYPE_TAKUM3) && node->expression()->is_typed(cdk::TYPE_BALANCED3))
      _pf.B2T();

    if (_function->is_typed(cdk::TYPE_BALANCED3))
    {
      _pf.STFVAL64I(); // integer: 8 bytes
    }
    else if (_function->is_typed(cdk::TYPE_STRING) || _function->is_typed(cdk::TYPE_POINTER))
    {
      _pf.STFVAL32I(); // string/pointer: 4 bytes
    }
    else if (_function->is_typed(cdk::TYPE_TAKUM3))
    {
      // takum3 (16 bytes) is returned through the hidden pointer the caller
      // provided: copy the value into the buffer it points to
      _pf.LOCAL(_funcSretOffset); // address of the pointer slot in the frame
      _pf.LDINT();                // the pointer itself (destination buffer)
      _pf.STTAKUM3();             // *pointer = value
    }
    else
    {
      std::cerr << node->lineno() << ": should not happen: unknown return type" << std::endl;
    }
  }
  _pf.JMP(mklbl(_funcEndLabel)); // single exit: jump to the function epilogue
}

void p6::postfix_writer::do_stop_node(p6::stop_node *const node, int lvl)
{
  // 'stop' breaks out of the loop: jump to its end label.
  // level() counts loops from the innermost (1) outwards.
  size_t level = node->level() == 0 ? 1 : node->level();
  if (level <= _whileEnd.size())
    _pf.JMP(mklbl(_whileEnd[_whileEnd.size() - level]));
}

void p6::postfix_writer::do_next_node(p6::next_node *const node, int lvl)
{
  // 'next' skips to the next iteration: jump back to the condition label.
  size_t level = node->level() == 0 ? 1 : node->level();
  if (level <= _whileCond.size())
    _pf.JMP(mklbl(_whileCond[_whileCond.size() - level]));
}

//---------------------------------------------------------------------------

void p6::postfix_writer::do_while_node(p6::while_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  _pf.LABEL(mklbl(lbl1 = ++_lbl));
  node->condition()->accept(this, lvl);
  // P6 (ternário): condição verdadeira só se positiva (> 0).
  if (node->condition()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2I();
  _pf.INT(0);
  _pf.GT(); // 1 se positivo (verdadeiro), 0 caso contrário
  _pf.JZ(mklbl(lbl2 = ++_lbl));
  _whileCond.push_back(lbl1); // so 'next' jumps to the condition re-test
  _whileEnd.push_back(lbl2);  // so 'stop' jumps past the loop
  node->block()->accept(this, lvl + 2);
  _whileCond.pop_back();
  _whileEnd.pop_back();
  _pf.JMP(mklbl(lbl1));
  _pf.LABEL(mklbl(lbl2));
}

//---------------------------------------------------------------------------

void p6::postfix_writer::do_if_node(p6::if_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1;
  node->condition()->accept(this, lvl);
  // P6 (ternário): condição verdadeira só se positiva (> 0).
  if (node->condition()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2I();
  _pf.INT(0);
  _pf.GT(); // 1 se positivo (verdadeiro), 0 caso contrário
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
  // P6 (ternário): condição verdadeira só se positiva (> 0).
  if (node->condition()->is_typed(cdk::TYPE_BALANCED3))
    _pf.B2I();
  _pf.INT(0);
  _pf.GT(); // 1 se positivo (verdadeiro), 0 caso contrário
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->thenblock()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl2 = ++_lbl));
  _pf.LABEL(mklbl(lbl1));
  node->elseblock()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1 = lbl2));
}
