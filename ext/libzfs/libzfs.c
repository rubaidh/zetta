#include <ruby.h>

#ifdef HAVE_LIBZFS_H
  #include <libzfs.h>
#endif

static VALUE my_zpool_initialize(VALUE self, VALUE pool_name, VALUE libzfs_handle)
{
  libzfs_handle_t *libhandle;
  zpool_handle_t  *zpool_handle;
  
  Data_Get_Struct(libzfs_handle, libzfs_handle_t, libhandle);

  // FIXME: Should be +_canfail+ or not?
  zpool_handle = zpool_open_canfail(libhandle, StringValuePtr(pool_name));
  
  // FIXME: This is definitely got to be the wrong way to store the
  // handle to the pool, but so far it appears to work.  Trouble is that I
  // can't do it the way that Programming Ruby says, because that would
  // involve calling +zpool_open()+ in the +alloc+ function, but we don't
  // know the pool name there.
  rb_iv_set(self, "@handle", Data_Wrap_Struct(rb_funcall3(self, rb_intern("class"), 0, 0), 0, zpool_close, zpool_handle));

  return self;
}

static VALUE my_zpool_get_name(VALUE self)
{
  zpool_handle_t *zpool_handle;
  Data_Get_Struct(rb_iv_get(self, "@handle"), zpool_handle_t, zpool_handle);
  
  return rb_str_new2(zpool_get_name(zpool_handle));
}

static VALUE my_zpool_get_guid(VALUE self)
{
  zpool_handle_t *zpool_handle;
  Data_Get_Struct(rb_iv_get(self, "@handle"), zpool_handle_t, zpool_handle);
  
  return ULL2NUM(zpool_get_guid(zpool_handle));
}

static VALUE my_zpool_get_space_used(VALUE self)
{
  zpool_handle_t *zpool_handle;
  Data_Get_Struct(rb_iv_get(self, "@handle"), zpool_handle_t, zpool_handle);
  
  return ULL2NUM(zpool_get_space_used(zpool_handle));
}

static VALUE my_zpool_get_space_total(VALUE self)
{
  zpool_handle_t *zpool_handle;
  Data_Get_Struct(rb_iv_get(self, "@handle"), zpool_handle_t, zpool_handle);
  
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

  rb_define_method(cZpool, "initialize", my_zpool_initialize, 2);
  rb_define_method(cZpool, "name", my_zpool_get_name, 0);
  rb_define_method(cZpool, "guid", my_zpool_get_guid, 0);
  rb_define_method(cZpool, "space_used", my_zpool_get_space_used, 0);
  rb_define_method(cZpool, "space_total", my_zpool_get_space_total, 0);
}
