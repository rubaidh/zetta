#include <ruby.h>

#ifdef HAVE_LIBNVPAIR_H
  #include <libnvpair.h>
#endif

static VALUE my_nvlist_alloc(VALUE klass)
{
  nvlist_t *list;
  nvlist_alloc(&list, 0, 0); // FIXME: Verify this works.
  return Data_Wrap_Struct(klass, 0, nvlist_free, list);
}

void Init_nvlist()
{
  VALUE cNVList = rb_define_class("NVList", rb_cObject);

  rb_define_alloc_func(cNVList, my_nvlist_alloc);
  // rb_define_method(cNVList, "initialize", my_nvlist_initialize, 0);
}
