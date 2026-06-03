# Plano de ação — completar o compilador P6 (grupo 019)

> Reescrito a 2026-06-02 como **plano de ação por marcos**. Substitui a versão de
> 2026-06-01 (que ainda tratava os opcodes ternários como "provavelmente RTS" e
> usava opcodes binários como placeholder). Os opcodes ternários abaixo foram
> **confirmados** por inspeção de `/usr/include/cdk/emitters/basic_postfix_emitter.h`
> dentro do Docker (ver §3). Continua a valer a regra: **não consigo compilar
> localmente** (sem Docker/flex/bison/CDK neste shell) — compila e verifica
> sempre com `./build.sh clean all` e com o harness (§1).

---

## 1. Rede de segurança (ler primeiro)

Três regras invioláveis, aplicadas a **cada** alteração:

1. **O build nunca fica vermelho.** Vale 0.5 da pauta e é a base de tudo.
   Depois de cada bloco: `./build.sh clean all` (a partir de `/root/Proj_Comp`)
   tem de produzir o executável `p6`.
2. **Regressão por categoria.** O harness `co26-tests/co26/test.sh` compila →
   assembla → liga (`-lrts`) → corre → faz `diff` com o `expected/*.out`.
   Aceita filtro:
   - `test.sh A` (só a categoria A), `test.sh B-08` (um teste só),
     `test.sh --clean` (rebuild de raiz).
   - Workflow por marco: implementas → `test.sh <categoria nova>` → e **voltas a
     correr as categorias que já passavam** para garantir que não regrediste.
3. **Commit por marco verde** (gestão do projeto = 1 valor). Nunca acumules um
   dia de alterações sem commit. Mantém `*.o`, `p6`, `p6_parser.tab.*`,
   `p6_scanner.cpp`, `p6_parser.output` no `.gitignore`.

> ⚠️ O Docker/`test.sh`/`build.sh` corres **tu** — este shell não tem Docker.

**Ponto de partida (importante):** neste momento o corpo do `main` **não gera
código nenhum**, porque `do_program_node` chama `node->block()` mas
`do_block_node` está vazio. O `_main` atual é só `ENTER(0); return 0`. Por isso o
primeiro marco (M1) é a fundação que põe o programa a emitir código.

---

## 2. Estado atual

| Componente | Ficheiro | Estado |
|---|---|---|
| Análise lexical | `p6_scanner.l` | ✅ Completo |
| Análise sintáctica | `p6_parser.y` | ✅ Completo (`data_type`/`void_type`/`ptr_type`/`func_type`/`ret_type` separados) |
| Nós da AST | `ast/*.h` | ✅ Existem todos |
| XML writer | `targets/xml_writer.cpp` | ✅ Completo |
| Tabela de símbolos | `targets/symbol.h` | ⚠️ Falta `offset` + flag global e (funções) assinatura |
| Type checker | `targets/type_checker.cpp` | ⚠️ Funciona mas é leniente (nunca rejeita) — endurecer no M9 |
| **Postfix writer** | `targets/postfix_writer.cpp` | ❌ Meio migrado e inconsistente — é o grosso do trabalho |

Migrado e **correto**: `do_balanced3_node`/`do_takum3_node` (literais),
`do_unary_minus_node`, `do_mod_node`, aritmética same-type (add/sub/mul/div),
esqueleto de `if`/`while`/`if_else`, `do_string_node`.

Restos do Simple **errados** ou **vazios**: tudo o que mexe em tamanhos
(`rvalue`/`assignment`/`evaluation`/`write`), comparações, lógica, variáveis,
funções, ponteiros, `stop`/`next`, e `do_block_node` (vazio → nada emite).

---

## 3. Contrato por tipo (opcodes CONFIRMADOS)

P6 **não** usa os tipos binários do CDK por omissão. O CDK em uso
(`home:d4vid:co26`) é **custom** e tem opcodes ternários nativos:

| Tipo | Tag | Tam. | Literal | Load / Store | Aritmética | Neg | DUP / TRASH | Print / Read | Comparar |
|---|---|:--:|---|---|---|---|---|---|---|
| int | `TYPE_BALANCED3` | 8 | `BALANCED3` | `LDBALANCED3`/`STBALANCED3` | `BADD BSUB BMUL BDIV BMOD` | `BNEG` | `DUP64` / 8 | `balanced3_print`/`_read` | `balanced3_lt…` (RTS) |
| real | `TYPE_TAKUM3` | 16 | `TAKUM3` | `LDTAKUM3`/`STTAKUM3` | `TADD TSUB TMUL TDIV` | `TNEG` | `DUP128` / 16 | `takum3_print`/`_read` | `takum3_lt…` (RTS) |
| string | `TYPE_STRING` | 4 | `SSTRING`+`ADDR` | `LDINT`/`STINT` | — | — | `DUP32` / 4 | `prints` | — |
| ponteiro | `TYPE_POINTER` | 4 | `null`/`?`/`[n]` | `LDINT`/`STINT` | `ADD`/`SUB` binárias | — | `DUP32` / 4 | (não imprime) | `EQ`/`NE` binárias |

**Conversões (confirmadas):** `B2T` int→real · `T2B` real→int · `B2I`/`I2B`
int↔binário (ponteiros, índices, código de saída do `_main`) · `T2D`/`D2T`
real↔IEEE754 (para `extern` C) · também `DUP32/64/128`, `STFVAL32I/64I`,
`LDFVAL…`, `LOCAL`/`LOCV`, `ENTER`/`LEAVE`/`RET`/`CALL`/`TRASH`/`ALLOC`/`SP`.

**Lógica de Kleene:** a RTS tem `kleene_and/or/not` (família ternária). Usa estas
funções via `CALL` — ver §4.

### 3.1 Onde ver os métodos + lista de referência das operações

Há **duas fontes** distintas, e é importante não as confundir:

- **Opcodes do emissor** — métodos de `_pf` (`_pf.T2B()`, `_pf.LDBALANCED3()`…),
  sem argumentos exceto onde indicado. Definidos em
  **`/usr/include/cdk/emitters/basic_postfix_emitter.h`** (existe só **dentro do
  Docker**). Para os veres:
  ```bash
  # da raiz /root/Proj_Comp
  docker compose run --rm dev less /usr/include/cdk/emitters/basic_postfix_emitter.h
  # ou só os ternários/conversões:
  docker compose run --rm dev grep -niE "balanced3|takum3|2t|2b|2i|2d|dup|fval|local" \
        /usr/include/cdk/emitters/basic_postfix_emitter.h
  ```
- **Funções da RTS** — chamadas por nome: `_pf.CALL("balanced3_print")`. Estão na
  biblioteca de run-time e no **manual da RTS**. Para descobrir os nomes exatos:
  ```bash
  docker compose run --rm dev sh -c 'find /usr/include -iname "*rts*"; \
        nm -D /usr/lib64/librts.so 2>/dev/null || nm /usr/lib64/librts*.a' \
        | grep -iE "balanced3|takum3|kleene|print|read"
  ```

#### A. Conversões (opcodes — a tua pergunta)
Notação `X2Y` = converte o valor no topo da pilha de **X para Y**:

| Tens (topo da pilha) | Queres | Opcode | Quando usas em P6 |
|---|---|---|---|
| takum3 (real) | balanced3 (int) | **`T2B`** | **real → int** (a tua pergunta) |
| balanced3 (int) | takum3 (real) | `B2T` | int → real (promoção em mistos, M7) |
| balanced3 (int) | binário 32b | `B2I` | índices, aritmética de ponteiros, saída do `_main` |
| binário 32b | balanced3 (int) | `I2B` | `sizeof`, contagens → int ternário |
| takum3 (real) | double IEEE754 | `T2D` | passar real a função `extern` C |
| double IEEE754 | takum3 (real) | `D2T` | receber real de função `extern` C |

(Existem ainda `P2B/B2P`, `T2P/P2T`, `P2D/D2P`, `D2I/I2D`, `D2F/F2D` — `posit3`
não é usado em P6.)

#### B. Restantes opcodes do emissor (por categoria)
- **Literais:** `BALANCED3(v)`, `TAKUM3(v)`, `INT(n)`, `DOUBLE(d)`.
- **Load** (do endereço no topo): `LDBALANCED3` (8B), `LDTAKUM3` (16B), `LDINT` (4B str/ptr).
- **Store** (valor+endereço): `STBALANCED3`, `STTAKUM3`, `STINT`.
- **Aritmética int:** `BADD BSUB BMUL BDIV BMOD BNEG`.
- **Aritmética real:** `TADD TSUB TMUL TDIV TNEG`.
- **Aritmética binária (ponteiros):** `ADD SUB MUL` (32b).
- **Pilha:** `DUP32 DUP64 DUP128`, `TRASH(n)`, `ALLOC`, `SP`.
- **Função/frame:** `ENTER(n) LEAVE RET CALL(s)`; retorno `STFVAL32I/STFVAL64I` +
  `LDFVAL32I/LDFVAL64I`; locais `LOCAL(off)` (endereço) / `LOCV(off)` (valor).
- **Símbolos/endereços:** `ADDR(s) GLOBAL(s,t) EXTERN(s) LABEL(s) FUNC() OBJ()`.
- **Saltos:** `JMP(l) JZ(l) JNZ(l)`.
- **Segmentos:** `TEXT DATA RODATA BSS ALIGN`; inicializadores estáticos
  `SINT(v) SALLOC(n) SSTRING(s)` e ternários `SBALANCED3(v) STAKUM3(v)`.

#### C. Funções da RTS (via `_pf.CALL("nome")`)
Padrão de uso: empurrar argumento(s) → `_pf.CALL("...")` → `_pf.TRASH(tam_args)` →
recolher resultado com `_pf.LDFVAL...()`. Famílias `balanced3_*` e `takum3_*`
(prefixo = tipo; `posit3_*` existe mas não se usa):

| Operação | `balanced3_*` (int) | `takum3_*` (real) |
|---|---|---|
| imprimir / ler | `balanced3_print` / `balanced3_read` | `takum3_print` / `takum3_read` |
| converter de | `balanced3_from_int/_from_double/_from_string` | `takum3_from_…` |
| converter para | `balanced3_to_int/_to_double` | `takum3_to_…` |
| aritmética | `balanced3_add/_sub/_mul/_div/_neg` | `takum3_add/…` |
| comparações | `balanced3_eq/_ne/_lt/_le/_gt/_ge` | `takum3_eq/…` |
| Kleene | `kleene_and/_or/_not` (ou `balanced3_kleene_*`) | — |

Binárias do esqueleto Simple (continuam disponíveis): `printi readi printd readd
prints readln println`.

> ⚠️ A lista vem da inspeção do header e da RTS (2026-06-02), mas **valida os
> nomes/assinaturas exatos** com os comandos acima antes de colar — sobretudo o
> prefixo das funções Kleene e a forma como as comparações devolvem o resultado.

---

## 4. Dois pontos a confirmar no Docker (condicionam o M2)

1. **`KAND`/`KOR`/`KNOT`** — o inventário confirmado **não** os lista; só confirma
   funções RTS `kleene_and/or/not`. Antes de usar opcodes, faz dentro do
   container: `grep -iE "kand|kor|knot|kleene" /usr/include/cdk/emitters/basic_postfix_emitter.h`.
   Se não existirem, usa `CALL kleene_and`/`kleene_or`/`kleene_not` (RTS).
2. **Protocolo das comparações/print RTS** — confirma no **manual da RTS**:
   nomes exatos (`balanced3_print` vs `print_balanced3`?), ordem dos operandos
   nas comparações, e como volta o resultado (`LDFVAL…` vs pilha). Tudo o que
   está marcado `// ⚠️` no §6 depende disto.

---

## 5. Marcos (cada um mantém o build verde e desbloqueia categorias)

| Marco | O que implementas | Ficheiros | Passa a verde |
|:--:|---|---|---|
| **M1 — Fundação** | `do_block_node`; `rvalue`/`assignment`/`evaluation`/`write` por **tipo** (LD/ST/DUP/TRASH/print certos); `EXTERN` certos | `postfix_writer.cpp` | **A, H** (e fim do `exit(1)`) |
| **M2 — Aritmética + controlo** | comparações via RTS; `&&`/`\|\|`/`~` (curto-circuito + Kleene); `if`/`while` passam a funcionar | `postfix_writer.cpp` | **B, I, W, V, R**, parte de **C** |
| **M3 — Globais** | `do_variable_declaration_node` (global em DATA/BSS); tirar a criação de global de dentro do `assignment`; `auto` | `postfix_writer.cpp`, `type_checker.cpp` | **C, D** |
| **M4 — Locais + âmbitos + frame** | `symbol` com `offset`+`global`; `do_block_node` push/pop; `do_variable_node` `ADDR` vs `LOCAL`; `ENTER(framesize)` | `symbol.h`, `type_checker.cpp`, `postfix_writer.{cpp,h}`, *(frame_size_calculator)* | **E**, shadowing **K-01/J-07** |
| **M5 — Funções** | def (prólogo/epílogo, args como locais), decl (`EXTERN`), call (Cdecl), `return` (ponteiro escondido p/ real) | `postfix_writer.{cpp,h}`, `symbol.h`, `type_checker.cpp`, *(p6_parser.y se preciso)* | **J, K, L, Y** |
| **M6 — stop/next** | pilhas de labels de ciclo; `while` empilha/desempilha | `postfix_writer.{cpp,h}` | **F** |
| **M7 — Reais + mistos** | promoção `B2T` (int→real) na aritmética/atribuição/args/return; comparações takum3 | `postfix_writer.cpp` | **M, N, O, T** |
| **M8 — Ponteiros** | `index` (binário, `B2I`), `address_of`, `stack_alloc`, `sizeof`, `null` | `postfix_writer.cpp` | **G, P, Q** |
| **M9 — Semântica rígida** | `type_checker` rejeita com `exit(2)`: não declarados, compatibilidade, return, aridade/tipos de chamadas, regras de `void` | `type_checker.cpp` | testes de erro |

**Porquê esta ordem mantém tudo a funcionar:**
- M1 é obrigatoriamente primeiro: sem `do_block_node` nenhum programa produz
  output, logo não há baseline para proteger.
- Cada marco seguinte **acrescenta** caminhos por tipo/nó; não reescreve o que o
  anterior pôs a funcionar. As únicas mudanças estruturais são tirar a gambiarra
  de global do `assignment` (M3) e introduzir offsets (M4) — por isso variáveis
  vêm antes de funções (que dependem de args-como-locais).
- A categoria **I** (inteiros > int32) entra logo no M2 de propósito: prova que
  migraste para ternário (8 bytes) e não ficaste no `LDINT` binário de 32 bits.

---

## 6. Templates de código (por marco, com opcodes confirmados)

> Esqueleto representativo. `// ⚠️` = confirmar nome/protocolo exato na RTS (§4).

### 6.1 M1 — fundação (corpo a emitir, modelo de valor por tipo)

```cpp
void p6::postfix_writer::do_block_node(p6::block_node *const node, int lvl) {
  // no M1 ainda sem scopes (só globais); o push/pop entra no M4
  if (node->declarations()) node->declarations()->accept(this, lvl + 2);
  if (node->instructions()) node->instructions()->accept(this, lvl + 2);
}

void p6::postfix_writer::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
  if      (node->is_typed(cdk::TYPE_TAKUM3))    _pf.LDTAKUM3();     // real 16B
  else if (node->is_typed(cdk::TYPE_BALANCED3)) _pf.LDBALANCED3();  // int  8B
  else                                          _pf.LDINT();        // str/ptr 4B
}

void p6::postfix_writer::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->rvalue()->accept(this, lvl);
  // int→real: se o lvalue é real e o rvalue é int, promover (ver M7)
  if (node->is_typed(cdk::TYPE_TAKUM3) &&
      node->rvalue()->is_typed(cdk::TYPE_BALANCED3)) _pf.B2T();

  if      (node->is_typed(cdk::TYPE_TAKUM3))    { _pf.DUP128(); }
  else if (node->is_typed(cdk::TYPE_BALANCED3)) { _pf.DUP64(); }
  else                                          { _pf.DUP32(); }

  node->lvalue()->accept(this, lvl);

  if      (node->is_typed(cdk::TYPE_TAKUM3))    _pf.STTAKUM3();
  else if (node->is_typed(cdk::TYPE_BALANCED3)) _pf.STBALANCED3();
  else                                          _pf.STINT();
}

void p6::postfix_writer::do_evaluation_node(p6::evaluation_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  if      (node->argument()->is_typed(cdk::TYPE_TAKUM3))    _pf.TRASH(16);
  else if (node->argument()->is_typed(cdk::TYPE_BALANCED3)) _pf.TRASH(8);
  else                                                      _pf.TRASH(4); // str/ptr
}

void p6::postfix_writer::do_write_node(p6::write_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  for (size_t i = 0; i < node->arguments()->size(); i++) {
    auto arg = dynamic_cast<cdk::expression_node *>(node->arguments()->node(i));
    arg->accept(this, lvl);
    if (arg->is_typed(cdk::TYPE_TAKUM3))        { _pf.CALL("takum3_print");    _pf.TRASH(16); } // ⚠️ nome RTS
    else if (arg->is_typed(cdk::TYPE_BALANCED3)){ _pf.CALL("balanced3_print"); _pf.TRASH(8);  } // ⚠️ nome RTS
    else                                        { _pf.CALL("prints");          _pf.TRASH(4);  }
  }
  if (node->newline()) _pf.CALL("println");
}
```

`do_program_node` no M1: mantém `ENTER(0)` (locais só no M4) e externa o que usas
mesmo:

```cpp
_pf.EXTERN("balanced3_print"); _pf.EXTERN("takum3_print");  // ⚠️ nomes RTS
_pf.EXTERN("prints"); _pf.EXTERN("println");
// (readi/balanced3_read só quando implementares input)
```

`do_balanced3_node`/`do_takum3_node` **já estão corretos** (`_pf.BALANCED3(...)`,
`_pf.TAKUM3(...)`). `do_integer_node`/`do_double_node`/`do_posit3_node` podem
ficar como estão — o parser nunca os cria.

### 6.2 M2 — comparações, lógica

```cpp
// Comparações: NÃO há opcode ternário; vai pela RTS.
void p6::postfix_writer::do_lt_node(cdk::lt_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  // ⚠️ confirmar: ordem dos operandos e recolha do resultado (LDFVAL?)
  _pf.CALL("balanced3_lt");      // takum3_lt se operandos reais
  _pf.TRASH(8 + 8);              // ⚠️ limpar os 2 operandos (tamanho conforme tipo)
  _pf.LDFVAL32I();               // ⚠️ resultado lógico
}
// le/gt/ge/eq/ne idênticos, trocando a função RTS.

// && com curto-circuito + Kleene (||/~ análogos).
void p6::postfix_writer::do_and_node(cdk::and_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl = ++_lbl;
  node->left()->accept(this, lvl);
  _pf.DUP64();                    // int = balanced3 (8B)
  _pf.JZ(mklbl(lbl));            // ⚠️ "falso" = 0; em Kleene U não é falso — confirmar V-*
  _pf.TRASH(8);
  node->right()->accept(this, lvl);
  _pf.CALL("kleene_and");        // ⚠️ combinar tri-valorado (RTS)
  _pf.LABEL(mklbl(lbl));
}
```

> ⚠️ A lógica de Kleene é **tri-valorada** (testes V: T / U / F). O curto-circuito
> e a representação de U têm de bater com os `.out` da categoria V — ajusta a
> condição de salto e usa `kleene_and/or/not` para a combinação.

### 6.3 M3/M4 — variáveis (global vs local), símbolo, frame

`symbol.h` (acrescentar campos):

```cpp
int  _offset = 0;     // offset no frame (válido só para locais/args)
bool _global = false; // true ⇒ DATA/BSS; false ⇒ frame
int  _qualifier = 0;  // public/forward/extern/private
// + getters/setters: offset(), offset(int), global(), global(bool), qualifier()...
```

`postfix_writer.h` (estado novo):

```cpp
bool _inFunctionBody = false;
int  _offset = 0;                       // próximo offset livre (negativo) p/ locais
std::shared_ptr<p6::symbol> _function;  // função atual (nullptr em globais)
int  _returnLabel = 0;
std::vector<int> _whileCond, _whileEnd; // M6
```

```cpp
void p6::postfix_writer::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  auto s = _symtab.find(node->name());
  if (s->global()) _pf.ADDR(s->name());
  else             _pf.LOCAL(s->offset());
}

void p6::postfix_writer::do_variable_declaration_node(
        p6::variable_declaration_node *const node, int lvl) {
  auto type = node->type();
  if (!type && node->initializer()) { ASSERT_SAFE_EXPRESSIONS; type = node->initializer()->type(); }
  auto s = std::make_shared<p6::symbol>(type, node->identifier(), 0);

  if (_inFunctionBody) {                       // LOCAL
    _offset -= type->size();
    s->offset(_offset); s->global(false);
    _symtab.insert(node->identifier(), s);
    if (node->initializer()) {                 // = atribuição ao endereço local
      node->initializer()->accept(this, lvl);
      // promoção int→real se preciso (M7)
      _pf.LOCAL(s->offset());
      if      (s->is_typed(cdk::TYPE_TAKUM3))    _pf.STTAKUM3();
      else if (s->is_typed(cdk::TYPE_BALANCED3)) _pf.STBALANCED3();
      else                                       _pf.STINT();
    }
  } else {                                     // GLOBAL
    s->global(true);
    _symtab.insert(node->identifier(), s);
    if (node->qualifier() == p6::QUALIFIER_FORWARD ||
        node->qualifier() == p6::QUALIFIER_EXTERN) return;     // só declara
    _pf.DATA(); _pf.ALIGN();
    if (node->qualifier() == p6::QUALIFIER_PUBLIC) _pf.GLOBAL(node->identifier(), _pf.OBJ());
    _pf.LABEL(node->identifier());
    if (node->initializer()) {
      // global inicializa com LITERAL (manual). Ex.: SBALANCED3(valor) / STAKUM3(...) / SSTRING(...)
    } else {
      _pf.SALLOC(type->size());                // espaço a zero
    }
  }
}
```

`do_block_node` (M4) passa a abrir/fechar âmbito: `_symtab.push(); … _symtab.pop();`.

Frame (`ENTER`): o padrão CDK é um visitor `frame_size_calculator` que soma os
`type()->size()` das declarações da função (incluindo blocos aninhados). Usa-o se
existir no CDK; senão, uma versão própria que desce a `if`/`while`/`{}`.

### 6.4 M5 — funções (def/decl/call/return)

```cpp
void p6::postfix_writer::do_function_definition_node(
        p6::function_definition_node *const node, int lvl) {
  auto function = std::make_shared<p6::symbol>(node->type(), node->identifier(), 0);
  function->global(true);
  _symtab.insert(node->identifier(), function);
  _function = function; _offset = 0;
  _symtab.push();

  int argoffset = 8;                            // FP guardado + endereço de retorno
  if (node->arguments())
    for (size_t i = 0; i < node->arguments()->size(); i++) {
      auto a = dynamic_cast<p6::variable_declaration_node*>(node->arguments()->node(i));
      auto s = std::make_shared<p6::symbol>(a->type(), a->identifier(), 0);
      s->offset(argoffset); s->global(false);
      _symtab.insert(a->identifier(), s);
      argoffset += a->type()->size();
    }

  _pf.TEXT(); _pf.ALIGN();
  if (node->qualifier() == p6::QUALIFIER_PUBLIC) _pf.GLOBAL(node->identifier(), _pf.FUNC());
  _pf.LABEL(node->identifier());
  _pf.ENTER(/* frame_size(node->block()) */ 0);

  _returnLabel = ++_lbl;
  _inFunctionBody = true;
  node->block()->accept(this, lvl + 2);
  _inFunctionBody = false;

  _pf.LABEL(mklbl(_returnLabel));
  _pf.LEAVE(); _pf.RET();
  _symtab.pop(); _function = nullptr;
}

void p6::postfix_writer::do_function_call_node(
        p6::function_call_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  auto symbol = _symtab.find(node->identifier());
  size_t argsbytes = 0;
  if (node->arguments())
    for (int i = node->arguments()->size() - 1; i >= 0; i--) {   // direita → esquerda
      auto arg = dynamic_cast<cdk::expression_node*>(node->arguments()->node(i));
      arg->accept(this, lvl + 2);
      // promover int→real se o formal for real; converter p/ C nos extern
      argsbytes += arg->type()->size();
    }
  _pf.CALL(node->identifier());
  if (argsbytes) _pf.TRASH(argsbytes);          // Cdecl: chamador limpa
  if (!(symbol && symbol->is_typed(cdk::TYPE_VOID))) {
    if      (node->is_typed(cdk::TYPE_TAKUM3))    { /* retorno real: ponteiro escondido (>64b) */ }
    else if (node->is_typed(cdk::TYPE_BALANCED3)) _pf.LDFVAL64I();   // ⚠️ confirmar
    else                                          _pf.LDFVAL32I();
  }
}

void p6::postfix_writer::do_return_node(p6::return_node *const node, int lvl) {
  if (node->expression()) {
    node->expression()->accept(this, lvl + 2);
    // converter para o tipo de retorno da função; no _main: B2I antes de STFVAL32I
    if (node->expression()->is_typed(cdk::TYPE_TAKUM3)) { /* escrever no ponteiro escondido */ }
    else if (node->expression()->is_typed(cdk::TYPE_BALANCED3)) _pf.STFVAL64I(); // ⚠️ confirmar
    else _pf.STFVAL32I();
  }
  _pf.JMP(mklbl(_returnLabel));
}
```

`do_function_declaration_node`: `forward`/`extern` só regista o símbolo (com a
assinatura completa) e emite `EXTERN`; não há corpo.

> **Decisão de design em aberto:** uniformiza para que o símbolo guarde sempre o
> **tipo funcional completo** (retorno + tipos dos argumentos). Hoje
> `function_definition_node.type()` devolve só o retorno → impede validar
> chamadas no M9 e faz `forward`/definição discordarem.

### 6.5 M6 — stop/next + while com pilhas

```cpp
void p6::postfix_writer::do_while_node(p6::while_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int cond = ++_lbl, end = ++_lbl;
  _whileCond.push_back(cond); _whileEnd.push_back(end);
  _pf.LABEL(mklbl(cond));
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(end));
  node->block()->accept(this, lvl + 2);
  _pf.JMP(mklbl(cond));
  _pf.LABEL(mklbl(end));
  _whileCond.pop_back(); _whileEnd.pop_back();
}

void p6::postfix_writer::do_stop_node(p6::stop_node *const node, int lvl) {
  size_t n = node->level();                              // n-ésimo ciclo (1 = mais interior)
  if (n >= 1 && n <= _whileEnd.size()) _pf.JMP(mklbl(_whileEnd[_whileEnd.size() - n]));
}
void p6::postfix_writer::do_next_node(p6::next_node *const node, int lvl) {
  size_t n = node->level();
  if (n >= 1 && n <= _whileCond.size()) _pf.JMP(mklbl(_whileCond[_whileCond.size() - n]));
}
```

### 6.6 M7 — promoção int→real (mistos)

Sempre que um operando é `TYPE_BALANCED3` mas o resultado/destino é
`TYPE_TAKUM3`, insere `_pf.B2T()` **depois de avaliar esse operando**. Aplica em:
`add/sub/mul/div` (avaliar cada lado, promover o que for int), `assignment`
(já no template M1), argumentos de chamada e `return`. Exemplo no `add`:

```cpp
void p6::postfix_writer::do_add_node(cdk::add_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_TAKUM3) && node->left()->is_typed(cdk::TYPE_BALANCED3)) _pf.B2T();
  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_TAKUM3) && node->right()->is_typed(cdk::TYPE_BALANCED3)) _pf.B2T();
  if (node->is_typed(cdk::TYPE_TAKUM3)) _pf.TADD();
  else _pf.BADD();  // ⚠️ ponteiro: ver M8 (aritmética binária, não BADD)
}
```

### 6.7 M8 — ponteiros e memória

```cpp
void p6::postfix_writer::do_address_of_node(p6::address_of_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl + 2);   // visitar o lvalue deixa o ENDEREÇO (4B)
}

void p6::postfix_writer::do_index_node(p6::index_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->base()->accept(this, lvl);         // ponteiro base (4B binário)
  node->index()->accept(this, lvl);        // índice é balanced3 (8B)...
  _pf.B2I();                               // ...converter p/ binário 32 bits
  _pf.INT(node->type()->size());           // tamanho do objeto apontado
  _pf.MUL(); _pf.ADD();                    // base + idx*size → endereço (lvalue)
}

void p6::postfix_writer::do_stack_alloc_node(p6::stack_alloc_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);     // nº de objetos (balanced3)
  _pf.B2I();
  _pf.INT(/* size do tipo referenciado */ 4);
  _pf.MUL(); _pf.ALLOC(); _pf.SP();        // reserva na pilha + ponteiro
}

void p6::postfix_writer::do_sizeof_node(p6::sizeof_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  _pf.INT(node->expression()->type()->size());
  _pf.I2B();                               // sizeof devolve int (balanced3) → 8B
}

void p6::postfix_writer::do_null_node(p6::null_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  _pf.INT(0);                              // ponteiro nulo = 0 (4B)
}
```

### 6.8 M9 — endurecer o type_checker (saída 2)

No `type_checker.cpp`, lançar `std::string` (a `main` converte em `exit(2)`):
não declarados (`do_variable_node`/`do_function_call_node` em vez de inventar
`int_type`), compatibilidade de operandos (`+ - * /` só int/real/ponteiro; `%`,
`~`, `&&`, `||` só int; string fora de aritmética), atribuição (`int→real` ok,
`null→[ponteiro]` ok, resto coincide), `return` vs tipo da função, aridade/tipos
das chamadas, indexar só ponteiros, não imprimir ponteiros. Corrigir também
`p - q` (dois ponteiros) → resultado **int**, não ponteiro.

---

## 7. Onde o manual diz que int = balanced3 e real = Takum3

**Secção "Tipos de Dados" → "Tipos numéricos"** (afirmação fundadora):

> "os **inteiros são ternários equilibrados**, ocupam 40 trits (empacotados em 64
> bits); os **reais são ternários e estão representados no formato Takum** (…),
> ocupam 80 trits (empacotados em 128 bits)."

- "ternários equilibrados" = *balanced ternary* → tipo CDK `cdk::balanced3_type`
  → `TYPE_BALANCED3` (40 trits / 64 bits = **8 bytes**).
- "formato Takum" → tipo CDK `cdk::takum3_type` → `TYPE_TAKUM3` (80 trits / 128
  bits = **16 bytes**).

**Reforços no resto do manual:**
- Secção dos literais, cabeçalho **"Reais em formato Takum3"** — usa
  literalmente o nome **Takum3**.
- Secção dos literais inteiros: **"Literais inteiros em base 3 (equilibrada)
  começam sempre com a sequência `0t`…"** — confirma a base ternária equilibrada
  dos inteiros.
- "alguns dos tipos de dados são ternários e devem ser manipulados com as
  **funções das ALUs**" — justifica o uso das funções RTS para print/compare.

> Nota para o relatório: o manual escreve "ternários equilibrados" (não a string
> "balanced3", que é o nome do tipo no CDK) e "Takum"/"Takum3". Cita assim para
> seres rigoroso.

---

## 8. Build, testes e gestão

- **Compilar:** `./build.sh clean all` a partir de `/root/Proj_Comp` (corre `make`
  no Docker; localmente não há flex/bison/CDK). Conflitos do bison →
  `p6_parser.output`.
- **Testar:** `co26-tests/co26/test.sh [categoria|teste] [--clean]` — 185 `.p6`
  com `expected/*.out` (185 programas; ~30 `.out` extra são casos de erro → M9).
- **Gestão (1 valor):** commit por marco verde; `.gitignore` com os artefactos de
  build; `./build.sh clean all` tem de funcionar de raiz.

### Como o compilador é avaliado (níveis 1 a 5)

O grader oficial do co26 não dá um simples PASS/FAIL: para cada teste avalia a
**pipeline por etapas** e marca até onde cada teste chegou. Cada nível só conta se
todos os anteriores passaram (são cumulativos):

| Nível | Etapa | O que confirma |
|-------|-------|----------------|
| **1** | `.p6` → `.asm` | o compilador `p6` produz ficheiro ASM sem rebentar |
| **2** | `.asm` → `.o` | o assembler (yasm/nasm) monta o objeto a partir do ASM |
| **3** | `.o` → executável | o linker liga o `.o` com a librts e gera o executável |
| **4** | execução | o programa corre e **termina com código de saída 0** |
| **5** | output | o output **coincide com o esperado** (ignora tab, espaço e `\n`) |

`OK 1 2 3 4` (sem o 5) é o caso típico de um corpo **vazio mas válido**: a
pipeline está intacta mas o programa não produz o output certo. A **pontuação só
depende do nível 5** — por isso é possível ter 185/185 "OK 1 2 3 4" e **0 pontos**.

> **Convenção de trabalho:** sempre que eu pedir para correr os testes, reporta logo
> os 5 níveis. Como ainda estou a mexer em partes internas, o que me interessa é
> saber se **quebrei a pipeline**: se um teste der 0 no nível 5, diz-me em que nível
> (1, 2, 3 ou 4) ele ficou — assim sei se a regressão foi no codegen do ASM, no
> assembler, no linker, ou em runtime (exit code ≠ 0).
