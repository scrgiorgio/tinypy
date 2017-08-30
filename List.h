#if 1
inline void _tp_list_realloc(tp_vm *tp, _tp_list *self, int len) {
  if (!len) { len = 1; }
  self->items = (tp_obj*)realloc(self->items, len * sizeof(tp_obj));
  self->alloc = len;
}

inline void _tp_list_set(tp_vm *tp, _tp_list *self, int k, tp_obj v, const char *error) {
  if (k >= self->len) {
    tp_raise(, tp_string("(_tp_list_set) KeyError"));
  }
  self->items[k] = v;
  tp_grey(tp, v);
}

inline void _tp_list_free(tp_vm *tp, _tp_list *self) {
  free(self->items);
  free(self);
}

inline tp_obj _tp_list_get(tp_vm *tp, _tp_list *self, int k, const char *error) {
  if (k >= self->len) {
    tp_raise(tp_None, tp_string("(_tp_list_set) KeyError"));
  }
  return self->items[k];
}

inline void _tp_list_insertx(tp_vm *tp, _tp_list *self, int n, tp_obj v) {
  if (self->len >= self->alloc) {
    _tp_list_realloc(tp, self, self->alloc * 2);
  }
  if (n < self->len) { memmove(&self->items[n + 1], &self->items[n], sizeof(tp_obj)*(self->len - n)); }
  self->items[n] = v;
  self->len += 1;
}

inline void _tp_list_appendx(tp_vm *tp, _tp_list *self, tp_obj v) {
  _tp_list_insertx(tp, self, self->len, v);
}

inline void _tp_list_insert(tp_vm *tp, _tp_list *self, int n, tp_obj v) {
  _tp_list_insertx(tp, self, n, v);
  tp_grey(tp, v);
}

inline void _tp_list_append(tp_vm *tp, _tp_list *self, tp_obj v) {
  _tp_list_insert(tp, self, self->len, v);
}

inline tp_obj _tp_list_pop(tp_vm *tp, _tp_list *self, int n, const char *error) {
  tp_obj r = _tp_list_get(tp, self, n, error);
  if (n != self->len - 1) { memmove(&self->items[n], &self->items[n + 1], sizeof(tp_obj)*(self->len - (n + 1))); }
  self->len -= 1;
  return r;
}

inline int _tp_list_find(tp_vm *tp, _tp_list *self, tp_obj v) {
  int n;
  for (n = 0; n < self->len; n++) {
    if (tp_cmp(tp, v, self->items[n]) == 0) {
      return n;
    }
  }
  return -1;
}

inline tp_obj tp_index(tp_vm *tp) {
  tp_obj self = TP_OBJ();
  tp_obj v = TP_OBJ();
  int i = _tp_list_find(tp, self.list.val, v);
  if (i < 0) {
    tp_raise(tp_None, tp_string("(tp_index) ValueError: list.index(x): x not in list"));
  }
  return tp_number(i);
}

inline _tp_list *_tp_list_new(tp_vm *tp) {
  return (_tp_list*)calloc(sizeof(_tp_list), 1);
}

inline tp_obj _tp_list_copy(tp_vm *tp, tp_obj rr) {
  tp_obj val = { TP_LIST };
  _tp_list *o = rr.list.val;
  _tp_list *r = _tp_list_new(tp);
  *r = *o; r->gci = 0;
  r->items = (tp_obj*)calloc(sizeof(tp_obj)*o->len, 1);
  memcpy(r->items, o->items, sizeof(tp_obj)*o->len);
  val.list.val = r;
  return tp_track(tp, val);
}

inline tp_obj tp_append(tp_vm *tp) {
  tp_obj self = TP_OBJ();
  tp_obj v = TP_OBJ();
  _tp_list_append(tp, self.list.val, v);
  return tp_None;
}

inline tp_obj tp_pop(tp_vm *tp) {
  tp_obj self = TP_OBJ();
  return _tp_list_pop(tp, self.list.val, self.list.val->len - 1, "pop");
}

inline tp_obj tp_insert(tp_vm *tp) {
  tp_obj self = TP_OBJ();
  int n = TP_NUM();
  tp_obj v = TP_OBJ();
  _tp_list_insert(tp, self.list.val, n, v);
  return tp_None;
}

inline tp_obj tp_extend(tp_vm *tp) {
  tp_obj self = TP_OBJ();
  tp_obj v = TP_OBJ();
  int i;
  for (i = 0; i < v.list.val->len; i++) {
    _tp_list_append(tp, self.list.val, v.list.val->items[i]);
  }
  return tp_None;
}

inline tp_obj tp_list_nt(tp_vm *tp) {
  tp_obj r = { TP_LIST };
  r.list.val = _tp_list_new(tp);
  return r;
}

inline tp_obj tp_list(tp_vm *tp) {
  tp_obj r = { TP_LIST };
  r.list.val = _tp_list_new(tp);
  return tp_track(tp, r);
}

inline tp_obj tp_list_n(tp_vm *tp, int n, tp_obj *argv) {
  int i;
  tp_obj r = tp_list(tp); _tp_list_realloc(tp, r.list.val, n);
  for (i = 0; i < n; i++) {
    _tp_list_append(tp, r.list.val, argv[i]);
  }
  return r;
}

inline int _tp_sort_cmp(tp_obj *a, tp_obj *b) {
  return tp_cmp(0, *a, *b);
}

inline tp_obj tp_sort(tp_vm *tp) {
  tp_obj self = TP_OBJ();
  qsort(self.list.val->items, self.list.val->len, sizeof(tp_obj), (int(*)(const void*, const void*))_tp_sort_cmp);
  return tp_None;
}


#endif