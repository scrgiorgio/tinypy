#if 1
/*
* Create a new empty string of a certain size.
* Does not put it in for GC tracking, since contents should be
* filled after returning.
*/
inline tp_obj tp_string_t(tp_vm *tp, int n) {
  tp_obj r = tp_string_n(0, n);
  r.string.info = (_tp_string*)calloc(sizeof(_tp_string) + n, 1);
  r.string.info->len = n;
  r.string.val = r.string.info->s;
  return r;
}

/*
* Create a new string which is a copy of some memory.
* This is put into GC tracking for you.
*/
inline tp_obj tp_string_copy(tp_vm *tp, const char *s, int n) {
  tp_obj r = tp_string_t(tp, n);
  memcpy(r.string.info->s, s, n);
  return tp_track(tp, r);
}

/*
* Create a new string which is a substring slice of another STRING.
* Does not need to be put into GC tracking, as its parent is
* already being tracked (supposedly).
*/
inline tp_obj tp_string_sub(tp_vm *tp, tp_obj s, int a, int b) {
  int l = s.string.len;
  a = _tp_max(0, (a < 0 ? l + a : a)); b = _tp_min(l, (b < 0 ? l + b : b));
  tp_obj r = s;
  r.string.val += a;
  r.string.len = b - a;
  return r;
}

inline tp_obj tp_printf(tp_vm *tp, char const *fmt, ...) {
  int l;
  tp_obj r;
  char *s;
  va_list arg;
  va_start(arg, fmt);
  l = vsnprintf(NULL, 0, fmt, arg);
  r = tp_string_t(tp, l);
  s = r.string.info->s;
  va_end(arg);
  va_start(arg, fmt);
  vsprintf(s, fmt, arg);
  va_end(arg);
  return tp_track(tp, r);
}

inline int _tp_str_index(tp_obj s, tp_obj k)
{
  int i = 0;
  while ((s.string.len - i) >= k.string.len) {
    if (memcmp(s.string.val + i, k.string.val, k.string.len) == 0) {
      return i;
    }
    i += 1;
  }
  return -1;
}

inline tp_obj tp_join(tp_vm *tp) {
  tp_obj delim = TP_OBJ();
  tp_obj val = TP_OBJ();
  int l = 0, i;
  tp_obj r;
  char *s;
  for (i = 0; i < val.list.val->len; i++) {
    if (i != 0) { l += delim.string.len; }
    l += tp_str(tp, val.list.val->items[i]).string.len;
  }
  r = tp_string_t(tp, l);
  s = r.string.info->s;
  l = 0;
  for (i = 0; i < val.list.val->len; i++) {
    tp_obj e;
    if (i != 0) {
      memcpy(s + l, delim.string.val, delim.string.len); l += delim.string.len;
    }
    e = tp_str(tp, val.list.val->items[i]);
    memcpy(s + l, e.string.val, e.string.len); l += e.string.len;
  }
  return tp_track(tp, r);
}

inline tp_obj tp_split(tp_vm *tp) {
  tp_obj v = TP_OBJ();
  tp_obj d = TP_OBJ();
  tp_obj r = tp_list(tp);

  int i;
  while ((i = _tp_str_index(v, d)) != -1) {
    _tp_list_append(tp, r.list.val, tp_string_sub(tp, v, 0, i));
    v.string.val += i + d.string.len; v.string.len -= i + d.string.len;
  }
  _tp_list_append(tp, r.list.val, tp_string_sub(tp, v, 0, v.string.len));
  return r;
}


inline tp_obj tp_find(tp_vm *tp) {
  tp_obj s = TP_OBJ();
  tp_obj v = TP_OBJ();
  return tp_number(_tp_str_index(s, v));
}

inline tp_obj tp_str_index(tp_vm *tp) {
  tp_obj s = TP_OBJ();
  tp_obj v = TP_OBJ();
  int n = _tp_str_index(s, v);
  if (n >= 0) { return tp_number(n); }
  tp_raise(tp_None, tp_string("(tp_str_index) ValueError: substring not found"));
}

inline tp_obj tp_str2(tp_vm *tp) {
  tp_obj v = TP_OBJ();
  return tp_str(tp, v);
}

inline tp_obj tp_chr(tp_vm *tp) {
  int v = TP_NUM();
  return tp_string_n(tp->chars[(unsigned char)v], 1);
}

inline tp_obj tp_ord(tp_vm *tp) {
  tp_obj s = TP_STR();
  if (s.string.len != 1) {
    tp_raise(tp_None, tp_string("(tp_ord) TypeError: ord() expected a character"));
  }
  return tp_number((unsigned char)s.string.val[0]);
}

inline tp_obj tp_strip(tp_vm *tp) {
  tp_obj o = TP_TYPE(TP_STRING);
  char const *v = o.string.val; int l = o.string.len;
  int i; int a = l, b = 0;
  tp_obj r;
  char *s;
  for (i = 0; i < l; i++) {
    if (v[i] != ' ' && v[i] != '\n' && v[i] != '\t' && v[i] != '\r') {
      a = _tp_min(a, i); b = _tp_max(b, i + 1);
    }
  }
  if ((b - a) < 0) { return tp_string(""); }
  r = tp_string_t(tp, b - a);
  s = r.string.info->s;
  memcpy(s, v + a, b - a);
  return tp_track(tp, r);
}

inline tp_obj tp_replace(tp_vm *tp) {
  tp_obj s = TP_OBJ();
  tp_obj k = TP_OBJ();
  tp_obj v = TP_OBJ();
  tp_obj p = s;
  int i, n = 0;
  int c;
  int l;
  tp_obj rr;
  char *r;
  char *d;
  tp_obj z;
  while ((i = _tp_str_index(p, k)) != -1) {
    n += 1;
    p.string.val += i + k.string.len; p.string.len -= i + k.string.len;
  }
  /*     fprintf(stderr,"ns: %d\n",n); */
  l = s.string.len + n * (v.string.len - k.string.len);
  rr = tp_string_t(tp, l);
  r = rr.string.info->s;
  d = r;
  z = p = s;
  while ((i = _tp_str_index(p, k)) != -1) {
    p.string.val += i; p.string.len -= i;
    memcpy(d, z.string.val, c = (p.string.val - z.string.val)); d += c;
    p.string.val += k.string.len; p.string.len -= k.string.len;
    memcpy(d, v.string.val, v.string.len); d += v.string.len;
    z = p;
  }
  memcpy(d, z.string.val, (s.string.val + s.string.len) - z.string.val);

  return tp_track(tp, rr);
}


#endif