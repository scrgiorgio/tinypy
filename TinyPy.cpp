
#include "tinypy.h"


#include "bytecode.h"

/* Function: tp_init
* Initializes a new virtual machine.
*
* The given parameters have the same format as the parameters to main, and
* allow passing arguments to your tinypy scripts.
*
* Returns:
* The newly created tinypy instance.
*/
tp_vm *tp_init(int argc, char *argv[])
{
  int i;
  tp_vm *tp = (tp_vm*)calloc(sizeof(tp_vm), 1);
  tp->cur = 0;
  tp->jmp = 0;
  tp->ex = tp_None;
  tp->root = tp_list_nt(tp);
  for (i = 0; i < 256; i++) { tp->chars[i][0] = i; }
  tp_gc_init(tp);
  tp->_regs = tp_list(tp);
  for (i = 0; i < TP_REGS; i++) { tp_set(tp, tp->_regs, tp_None, tp_None); }
  tp->builtins = tp_dict(tp);
  tp->modules = tp_dict(tp);
  tp->_params = tp_list(tp);
  for (i = 0; i < TP_FRAMES; i++) { tp_set(tp, tp->_params, tp_None, tp_list(tp)); }
  tp_set(tp, tp->root, tp_None, tp->builtins);
  tp_set(tp, tp->root, tp_None, tp->modules);
  tp_set(tp, tp->root, tp_None, tp->_regs);
  tp_set(tp, tp->root, tp_None, tp->_params);
  tp_set(tp, tp->builtins, tp_string("MODULES"), tp->modules);
  tp_set(tp, tp->modules, tp_string("BUILTINS"), tp->builtins);
  tp_set(tp, tp->builtins, tp_string("BUILTINS"), tp->builtins);
  tp_obj sys = tp_dict(tp);
  tp_set(tp, sys, tp_string("version"), tp_string("tinypy 1.2+SVN"));
  tp_set(tp, tp->modules, tp_string("sys"), sys);
  tp->regs = tp->_regs.list.val->items;
  tp_full(tp);

  tp_builtins(tp);
  tp_args(tp, argc, argv);

  tp_import(tp, 0, "tokenize", tp_tokenize, sizeof(tp_tokenize));
  tp_import(tp, 0, "parse"   , tp_parse   , sizeof(tp_parse));
  tp_import(tp, 0, "encode"  , tp_encode  , sizeof(tp_encode));
  tp_import(tp, 0, "py2bc"   , tp_py2bc   , sizeof(tp_py2bc));

  tp_ez_call(tp, "py2bc", "_init", tp_None);

  return tp;
}


tp_obj tp_None = { TP_NONE };

void math_init(tp_vm *tp);

int main(int argc, char *argv[])
{
  tp_vm *tp = tp_init(argc, argv);

  math_init(tp);

  tp_ez_call(tp, "py2bc", "tinypy", tp_None);
  tp_deinit(tp);
  return(0);
}

