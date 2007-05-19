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
  VALUE pool_name, libzfs_handle;
  libzfs_handle_t *libhandle;
  zpool_handle_t  *zpool_handle;

  if(argc != 2) {
    rb_raise(rb_eArgError, "Two arguments are required -- the pool name and libzfs handle.");
  }
  pool_name = argv[0];
  libzfs_handle = argv[1];
  
  Data_Get_Struct(libzfs_handle, libzfs_handle_t, libhandle);

  // FIXME: How do I switch from a symbol to a string automagically?
  // if(SYMBOL_P(pool_name)) {
  //   pool_name = rb_funcall(pool_name, 'to_s', 0);
  // }

  // FIXME: Should be +_canfail+ or not?
  zpool_handle = zpool_open_canfail(libhandle, StringValuePtr(pool_name));

  return Data_Wrap_Struct(klass, 0, zpool_close, zpool_handle);
}

static VALUE my_zpool_get_handle(VALUE self)
{
  VALUE klass = rb_const_get(rb_cObject, rb_intern("LibZfs"));
  libzfs_handle_t *handle;
  zpool_handle_t *zpool_handle;
  
  Data_Get_Struct(self, zpool_handle_t, zpool_handle);

  handle = zpool_get_handle(zpool_handle);

  // Note that we don't need to free the handle here, because it's just a
  // copy of one that's already in the garbage collector.
  return Data_Wrap_Struct(klass, 0, 0, handle);
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

static VALUE my_zpool_get_root(VALUE self)
{
  zpool_handle_t *zpool_handle;
  char root[MAXPATHLEN];
  Data_Get_Struct(self, zpool_handle_t, zpool_handle);

  if(zpool_get_root(zpool_handle, root, sizeof(root)) < 0) {
    return Qnil;
  } else {
    return rb_str_new2(root);
  }
}

static VALUE my_zpool_get_state(VALUE self)
{
  zpool_handle_t *zpool_handle;
  Data_Get_Struct(self, zpool_handle_t, zpool_handle);

  return INT2NUM(zpool_get_state(zpool_handle));
}

static VALUE my_zpool_get_version(VALUE self)
{
  zpool_handle_t *zpool_handle;
  Data_Get_Struct(self, zpool_handle_t, zpool_handle);
  
  return ULL2NUM(zpool_get_version(zpool_handle));
}

static int my_zpool_iter_f(zpool_handle_t *handle, void *klass)
{
  rb_yield(Data_Wrap_Struct((VALUE)klass, 0, zpool_close, handle));
  return 0;
}

static VALUE my_zpool_iter(VALUE klass, VALUE libzfs_handle)
{
  libzfs_handle_t *libhandle;
  Data_Get_Struct(libzfs_handle, libzfs_handle_t, libhandle);

  zpool_iter(libhandle, my_zpool_iter_f, (void *)klass);
  
  return Qnil;
}

// static VALUE my_zpool_create(VALUE libzfs_handle, VALUE name, VALUE vdevs, VALUE altroot)
// {
//   libzfs_handle_t *libhandle;
//   nvlist_t *vdevs = NULL;
//   Data_Get_Struct(libzfs_handle, libzfs_handle_t, libhandle);
//   
//   return INT2NUM(zpool_create(libzfs_handle, StringValuePtr(name), vdev_list, StringValuePtr(altroot)));
// }


/*
 * ZFS interface
 */

// TODO
// static VALUE my_zfs_get_handle(VALUE self)
// {
//   
// }


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

  libzfs_print_on_error(handle, RTEST(b));
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
  rb_define_method(cZpool, "root", my_zpool_get_root, 0);
  rb_define_method(cZpool, "state", my_zpool_get_state, 0);
  rb_define_method(cZpool, "version", my_zpool_get_version, 0);
  rb_define_method(cZpool, "libzfs_handle", my_zpool_get_handle, 0);
  
  rb_define_singleton_method(cZpool, "each", my_zpool_iter, 1);
}
