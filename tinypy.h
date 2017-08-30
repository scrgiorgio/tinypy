
#ifndef TINYPY_H__
#define TINYPY_H__

#ifdef _WIN32
#pragma warning(disable:4267 4244 4996)
#endif


#include <setjmp.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>

enum 
{
  TP_NONE, 
  TP_NUMBER, 
  TP_STRING, 
  TP_DICT,
  TP_LIST, 
  TP_FNC, 
  TP_DATA,
};

typedef struct tp_number_ 
{
  int    type;
  double val;
} 
tp_number_;

typedef struct tp_string_ 
{
  int                type;
  struct _tp_string *info;
  char const*        val;
  int                len;
} 
tp_string_;

typedef struct tp_list_ 
{
  int type;
  struct _tp_list *val;
} 
tp_list_;

typedef struct tp_dict_ 
{
  int type;
  struct _tp_dict *val;
  int dtype;
} 
tp_dict_;

typedef struct tp_fnc_ 
{
  int type;
  struct _tp_fnc *info;
  int ftype;
  void *cfnc;
} 
tp_fnc_;

typedef struct tp_data_ 
{
  int type;
  struct _tp_data *info;
  void *val;
  int magic;
} 
tp_data_;

/* Type: tp_obj
 * Tinypy's object representation.
 *
 * Every object in tinypy is of this type in the C API.
 *
 * Fields:
 * type - This determines what kind of objects it is. It is either TP_NONE, in
 *        which case this is the none type and no other fields can be accessed.
 *        Or it has one of the values listed below, and the corresponding
 *        fields can be accessed.
 * number - TP_NUMBER
 * number.val - A double value with the numeric value.
 * string - TP_STRING
 * string.val - A pointer to the string data.
 * string.len - Length in bytes of the string data.
 * dict - TP_DICT
 * list - TP_LIST
 * fnc - TP_FNC
 * data - TP_DATA
 * data.val - The user-provided data pointer.
 * data.magic - The user-provided magic number for identifying the data type.
 */
typedef union tp_obj 
{
  int type;
  tp_number_ number;
  struct { int type; int *data; } gci;
  tp_string_ string;
  tp_dict_ dict;
  tp_list_ list;
  tp_fnc_ fnc;
  tp_data_ data;
} 
tp_obj;

typedef struct _tp_string 
{
  int gci;
  int len;
  char s[1];
} 
_tp_string;

typedef struct _tp_list 
{
  int gci;
  tp_obj *items;
  int len;
  int alloc;
} 
_tp_list;

typedef struct tp_item 
{
  int used;
  int hash;
  tp_obj key;
  tp_obj val;
} 
tp_item;

typedef struct _tp_dict 
{
  int gci;
  tp_item *items;
  int len;
  int alloc;
  int cur;
  int mask;
  int used;
  tp_obj meta;
} 
_tp_dict;

typedef struct _tp_fnc 
{
  int gci;
  tp_obj self;
  tp_obj globals;
  tp_obj code;
}
_tp_fnc;


typedef union tp_code 
{
  unsigned char i;
  struct { unsigned char i, a, b, c; } regs;
  struct { char val[4]; } string;
  struct { float val; } number;
} 
tp_code;

typedef struct tp_frame_ 
{
  /*    tp_code *codes; */
  tp_obj code;
  tp_code *cur;
  tp_code *jmp;
  tp_obj *regs;
  tp_obj *ret_dest;
  tp_obj fname;
  tp_obj name;
  tp_obj line;
  tp_obj globals;
  int lineno;
  int cregs;
} 
tp_frame_;

#define TP_GCMAX      4096
#define TP_FRAMES     256
#define TP_REGS_EXTRA 2
#define TP_REGS       16384

/* Type: tp_vm
 * Representation of a tinypy virtual machine instance.
 *
 * A new tp_vm struct is created with <tp_init>, and will be passed to most
 * tinypy functions as first parameter. It contains all the data associated
 * with an instance of a tinypy virtual machine - so it is easy to have
 * multiple instances running at the same time. When you want to free up all
 * memory used by an instance, call <tp_deinit>.
 *
 * Fields:
 * These fields are currently documented:
 *
 * builtins - A dictionary containing all builtin objects.
 * modules - A dictionary with all loaded modules.
 * params - A list of parameters for the current function call.
 * frames - A list of all call frames.
 * cur - The index of the currently executing call frame.
 * frames[n].globals - A dictionary of global sybmols in callframe n.
 */
typedef struct tp_vm 
{
  tp_obj builtins;
  tp_obj modules;
  tp_frame_ frames[TP_FRAMES];
  tp_obj _params;
  tp_obj params;
  tp_obj _regs;
  tp_obj *regs;
  tp_obj root;
  jmp_buf buf;
  int jmp;
  tp_obj ex;
  char chars[256][2];
  int cur;
  /* gc */
  _tp_list *white;
  _tp_list *grey;
  _tp_list *black;
  int steps;

} 
tp_vm;

typedef struct _tp_data 
{
  int gci;
  void(*free)(tp_vm *tp, tp_obj);
}
_tp_data;

extern tp_obj tp_None;

void tp_sandbox(tp_vm *tp, double, unsigned long);
void tp_time_update(tp_vm *tp);
void tp_mem_update(tp_vm *tp);

void   tp_run(tp_vm *tp, int cur);
void   tp_set(tp_vm *tp, tp_obj, tp_obj, tp_obj);
tp_obj tp_get(tp_vm *tp, tp_obj, tp_obj);
tp_obj tp_has(tp_vm *tp, tp_obj self, tp_obj k);
tp_obj tp_len(tp_vm *tp, tp_obj);
void   tp_del(tp_vm *tp, tp_obj, tp_obj);
tp_obj tp_str(tp_vm *tp, tp_obj);
int    tp_bool(tp_vm *tp, tp_obj);
int    tp_cmp(tp_vm *tp, tp_obj, tp_obj);
void   _tp_raise(tp_vm *tp, tp_obj);
tp_obj tp_printf(tp_vm *tp, char const *fmt, ...);
tp_obj tp_track(tp_vm *tp, tp_obj);
void   tp_grey(tp_vm *tp, tp_obj);
tp_obj tp_call(tp_vm *tp, tp_obj fnc, tp_obj params);
tp_obj tp_add(tp_vm *tp, tp_obj a, tp_obj b);

/* __func__ __VA_ARGS__ __FILE__ __LINE__ */
/* Function: tp_raise
 * Macro to raise an exception.
 *
 * This macro will return from the current function returning "r". The
 * remaining parameters are used to format the exception message.
 */
#define tp_raise(r,v) { \
    _tp_raise(tp,v); \
    return r; \
}

 /* Function: tp_string
  * Creates a new string object from a C string.
  *
  * Given a pointer to a C string, creates a tinypy object representing the
  * same string.
  *
  * *Note* Only a reference to the string will be kept by tinypy, so make sure
  * it does not go out of scope, and don't de-allocate it. Also be aware that
  * tinypy will not delete the string for you. In many cases, it is best to
  * use <tp_string_t> or <tp_string_slice> to create a string where tinypy
  * manages storage for you.
  */
inline static tp_obj tp_string(char const *v) {
  tp_obj val;
  tp_string_ s = { TP_STRING, 0, v, 0 };
  s.len = (int)strlen(v);
  val.string = s;
  return val;
}

inline static void tp_cstr(tp_vm *tp, tp_obj v, char *s, int l) 
{
  if (v.type != TP_STRING) {
    tp_raise(, tp_string("(tp_cstr) TypeError: value not a string"));
  }
  if (v.string.len >= l) {
    tp_raise(, tp_string("(tp_cstr) TypeError: value too long"));
  }
  memset(s, 0, l);
  memcpy(s, v.string.val, v.string.len);
}


#define TP_OBJ() (tp_get(tp,tp->params,tp_None))

inline static tp_obj tp_type(tp_vm *tp, int t, tp_obj v) 
{
  if (v.type != t) { 
    tp_raise(tp_None, tp_string("(tp_type) TypeError: unexpected type")); 
  }
  return v;
}


#define TP_NO_LIMIT 0
#define TP_TYPE(t) tp_type(tp,t,TP_OBJ())
#define TP_NUM() (TP_TYPE(TP_NUMBER).number.val)
#define TP_STR() (TP_TYPE(TP_STRING))
#define TP_DEFAULT(d) (tp->params.list.val->len?tp_get(tp,tp->params,tp_None):(d))

/* Macro: TP_LOOP
 * Macro to iterate over all remaining arguments.
 *
 * If you have a function which takes a variable number of arguments, you can
 * iterate through all remaining arguments for example like this:
 *
 * > tp_obj *my_func(tp_vm *tp)
 * > {
 * >     // We retrieve the first argument like normal.
 * >     tp_obj first = TP_OBJ();
 * >     // Then we iterate over the remaining arguments.
 * >     tp_obj arg;
 * >     TP_LOOP(arg)
 * >         // do something with arg
 * >     TP_END
 * > }
 */
#define TP_LOOP(e) \
    int __l = tp->params.list.val->len; \
    int __i; for (__i=0; __i<__l; __i++) { \
    (e) = _tp_list_get(tp,tp->params.list.val,__i,"TP_LOOP");
#define TP_END \
    }

inline static int _tp_min(int a, int b) { return (a < b ? a : b); }
inline static int _tp_max(int a, int b) { return (a > b ? a : b); }
inline static int _tp_sign(double v)    { return (v < 0 ? -1 : (v > 0 ? 1 : 0)); }

/* Function: tp_number
 * Creates a new numeric object.
 */
inline static tp_obj tp_number(double v) {
  tp_obj val = { TP_NUMBER };
  val.number.val = v;
  return val;
}

inline static void tp_echo(tp_vm *tp, tp_obj e) {
  e = tp_str(tp, e);
  fwrite(e.string.val, 1, e.string.len, stdout);
}

/* Function: tp_string_n
 * Creates a new string object from a partial C string.
 *
 * Like <tp_string>, but you specify how many bytes of the given C string to
 * use for the string object. The *note* also applies for this function, as the
 * string reference and length are kept, but no actual substring is stored.
 */
inline static tp_obj tp_string_n(char const *v, int n) {
  tp_obj val;
  tp_string_ s = { TP_STRING, 0,v,n };
  val.string = s;
  return val;
}

#include "List.h"
#include "Dict.h"
#include "Misc.h"
#include "String.h"
#include "Builtins.h"
#include "GarbageCollect.h"
#include "Ops.h"
#include "VirtualMachine.h"

#endif //TINYPY_H__
