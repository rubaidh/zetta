#include <ruby.h>

#ifdef HAVE_LIBZFS_H
  #include <libzfs.h>
#endif

// We have to merge alloc and init here because we want to allocate the space
// for the C data structure, but we also need the arguments passed to
// initialize to do so.
// 
// FIXME: I'm still not sure this is the right way to do it!
static VALUE my_zpool_new(int argc, VALUE *argv, VALUE klass)
{
  // FIXME: Make this robust!
  VALUE pool_name = argv[0];
  VALUE libzfs_handle = argv[1];
  
  libzfs_handle_t *libhandle;
  zpool_handle_t  *zpool_handle;

  Data_Get_Struct(libzfs_handle, libzfs_handle_t, libhandle);

  // FIXME: Should be +_canfail+ or not?
  zpool_handle = zpool_open_canfail(libhandle, StringValuePtr(pool_name));

  return Data_Wrap_Struct(klass, 0, zpool_close, zpool_handle);
}

static VALUE my_zpool_get_name(VALUE self)
{
  zpool_handle_t *zpool_handle;
  Data_Get_Struct(self, zpool_handle_t, zpool_handle);
  
  return rb_str_new2(zpool_get_name(zpool_handle));
}

static VALUE my_zpool_get_guid(VALUE self)
{
  zpool_handle_t *zpool_handle;
  Data_Get_Struct(self, zpool_handle_t, zpool_handle);
  
  return ULL2NUM(zpool_get_guid(zpool_handle));
}

static VALUE my_zpool_get_space_used(VALUE self)
{
  zpool_handle_t *zpool_handle;
  Data_Get_Struct(self, zpool_handle_t, zpool_handle);
  
  return ULL2NUM(zpool_get_space_used(zpool_handle));
}

static VALUE my_zpool_get_space_total(VALUE self)
{
  zpool_handle_t *zpool_handle;
  Data_Get_Struct(self, zpool_handle_t, zpool_handle);
  
  return ULL2NUM(zpool_get_space_total(zpool_handle));
}

/*
 * The low-level libzfs handle widget.
 */
static VALUE my_libzfs_alloc(VALUE klass)
{
  libzfs_handle_t *handle = libzfs_init();
  return Data_Wrap_Struct(klass, 0, libzfs_fini, handle);
}

static VALUE my_libzfs_errno(VALUE self)
{
  libzfs_handle_t *handle;
  Data_Get_Struct(self, libzfs_handle_t, handle);
  return INT2NUM(libzfs_errno(handle));
}

static VALUE my_libzfs_print_on_error(VALUE self, VALUE b)
{
  libzfs_handle_t *handle;
  Data_Get_Struct(self, libzfs_handle_t, handle);

  libzfs_print_on_error(handle, b == Qtrue);
  return Qnil;
}

static VALUE my_libzfs_error_action(VALUE self)
{
  libzfs_handle_t *handle;
  Data_Get_Struct(self, libzfs_handle_t, handle);

  return rb_str_new2(libzfs_error_action(handle));
}

static VALUE my_libzfs_error_description(VALUE self)
{
  libzfs_handle_t *handle;
  Data_Get_Struct(self, libzfs_handle_t, handle);

  return rb_str_new2(libzfs_error_description(handle));
}

void Init_libzfs()
{
  VALUE cLibZfs = rb_define_class("LibZfs", rb_cObject);
  VALUE cZpool = rb_define_class("Zpool", rb_cObject);

  rb_define_alloc_func(cLibZfs, my_libzfs_alloc);
  rb_define_method(cLibZfs, "errno", my_libzfs_errno, 0);
  rb_define_method(cLibZfs, "print_on_error", my_libzfs_print_on_error, 1);
  rb_define_method(cLibZfs, "error_action", my_libzfs_error_action, 0);
  rb_define_method(cLibZfs, "error_description", my_libzfs_error_description, 0);

  rb_define_singleton_method(cZpool, "new", my_zpool_new, -1);
  rb_define_method(cZpool, "name", my_zpool_get_name, 0);
  rb_define_method(cZpool, "guid", my_zpool_get_guid, 0);
  rb_define_method(cZpool, "space_used", my_zpool_get_space_used, 0);
  rb_define_method(cZpool, "space_total", my_zpool_get_space_total, 0);
}
