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
  char root[ZPOOL_MAXNAMELEN];
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

// FIXME: Doesn't appear to work?  Maybe I actually need to offline it before
// I can destroy?  If so, that's something for the higher level Ruby library.
static VALUE my_zpool_destroy(VALUE self)
{
  zpool_handle_t *zpool_handle;
  Data_Get_Struct(self, zpool_handle_t, zpool_handle);
  
  return INT2NUM(zpool_destroy(zpool_handle));
}

/*
 * ZFS interface
 */

static VALUE my_zfs_get_handle(VALUE self)
{
  VALUE klass = rb_const_get(rb_cObject, rb_intern("LibZfs"));
  libzfs_handle_t *handle;
  zfs_handle_t *zfs_handle;
  
  Data_Get_Struct(self, zfs_handle_t, zfs_handle);

  handle = zfs_get_handle(zfs_handle);

  // Note that we don't need to free the handle here, because it's just a
  // copy of one that's already in the garbage collector.
  return Data_Wrap_Struct(klass, 0, 0, handle);
}

static VALUE my_zfs_new(int argc, VALUE *argv, VALUE klass)
{
  VALUE fs_name, libzfs_handle, types;
  libzfs_handle_t *libhandle;
  zfs_handle_t  *zfs_handle;

  if(argc != 3) {
    rb_raise(rb_eArgError, "Two arguments are required -- the file system name, libzfs handle and a mask of types.");
  }
  fs_name = argv[0];
  libzfs_handle = argv[1];
  types = argv[2];

  Data_Get_Struct(libzfs_handle, libzfs_handle_t, libhandle);
  zfs_handle = zfs_open(libhandle, StringValuePtr(fs_name), NUM2INT(types));
  return Data_Wrap_Struct(klass, 0, zfs_close, zfs_handle);
}

static VALUE my_zfs_is_shared(VALUE self)
{
  zfs_handle_t *zfs_handle;
  Data_Get_Struct(self, zfs_handle_t, zfs_handle);
  
  return zfs_is_shared(zfs_handle) ? Qtrue : Qfalse;
}

static VALUE my_zfs_share(VALUE self)
{
  zfs_handle_t *zfs_handle;
  Data_Get_Struct(self, zfs_handle_t, zfs_handle);
  
  return INT2NUM(zfs_share(zfs_handle));
}

static VALUE my_zfs_unshare(VALUE self)
{
  zfs_handle_t *zfs_handle;
  Data_Get_Struct(self, zfs_handle_t, zfs_handle);
  
  return INT2NUM(zfs_unshare(zfs_handle));
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

static void Init_libzfs_consts()
{
  VALUE cZfsConsts = rb_define_module("ZfsConsts");
  VALUE mErrors = rb_define_module_under(cZfsConsts, "Errors");
  VALUE mTypes = rb_define_module_under(cZfsConsts, "Types");
  VALUE mHealthStatus = rb_define_module_under(cZfsConsts, "HealthStatus");
  VALUE mState = rb_define_module_under(cZfsConsts, "State");
  VALUE mPoolState = rb_define_module_under(mState, "Pool");

  // Current version
  rb_define_const(cZfsConsts, "VERSION", INT2NUM(ZFS_VERSION));

  // Filesystem types
  rb_define_const(mTypes, "FILESYSTEM", INT2NUM(ZFS_TYPE_FILESYSTEM));
  rb_define_const(mTypes, "SNAPSHOT", INT2NUM(ZFS_TYPE_SNAPSHOT));
  rb_define_const(mTypes, "VOLUME", INT2NUM(ZFS_TYPE_VOLUME));
  rb_define_const(mTypes, "POOL", INT2NUM(ZFS_TYPE_POOL));
  rb_define_const(mTypes, "ANY", INT2NUM(ZFS_TYPE_ANY));

  // Error codes
  rb_define_const(mErrors, "NOMEM", INT2NUM(EZFS_NOMEM));
  rb_define_const(mErrors, "BADPROP", INT2NUM(EZFS_BADPROP));
  rb_define_const(mErrors, "PROPREADONLY", INT2NUM(EZFS_PROPREADONLY));
  rb_define_const(mErrors, "PROPTYPE", INT2NUM(EZFS_PROPTYPE));
  rb_define_const(mErrors, "PROPNONINHERIT", INT2NUM(EZFS_PROPNONINHERIT));
  rb_define_const(mErrors, "PROPSPACE", INT2NUM(EZFS_PROPSPACE));
  rb_define_const(mErrors, "BADTYPE", INT2NUM(EZFS_BADTYPE));
  rb_define_const(mErrors, "BUSY", INT2NUM(EZFS_BUSY));
  rb_define_const(mErrors, "EXISTS", INT2NUM(EZFS_EXISTS));
  rb_define_const(mErrors, "NOENT", INT2NUM(EZFS_NOENT));
  rb_define_const(mErrors, "BADSTREAM", INT2NUM(EZFS_BADSTREAM));
  rb_define_const(mErrors, "DSREADONLY", INT2NUM(EZFS_DSREADONLY));
  rb_define_const(mErrors, "VOLTOOBIG", INT2NUM(EZFS_VOLTOOBIG));
  rb_define_const(mErrors, "VOLHASDATA", INT2NUM(EZFS_VOLHASDATA));
  rb_define_const(mErrors, "INVALIDNAME", INT2NUM(EZFS_INVALIDNAME));
  rb_define_const(mErrors, "BADRESTORE", INT2NUM(EZFS_BADRESTORE));
  rb_define_const(mErrors, "BADBACKUP", INT2NUM(EZFS_BADBACKUP));
  rb_define_const(mErrors, "BADTARGET", INT2NUM(EZFS_BADTARGET));
  rb_define_const(mErrors, "NODEVICE", INT2NUM(EZFS_NODEVICE));
  rb_define_const(mErrors, "BADDEV", INT2NUM(EZFS_BADDEV));
  rb_define_const(mErrors, "NOREPLICAS", INT2NUM(EZFS_NOREPLICAS));
  rb_define_const(mErrors, "RESILVERING", INT2NUM(EZFS_RESILVERING));
  rb_define_const(mErrors, "BADVERSION", INT2NUM(EZFS_BADVERSION));
  rb_define_const(mErrors, "POOLUNAVAIL", INT2NUM(EZFS_POOLUNAVAIL));
  rb_define_const(mErrors, "DEVOVERFLOW", INT2NUM(EZFS_DEVOVERFLOW));
  rb_define_const(mErrors, "BADPATH", INT2NUM(EZFS_BADPATH));
  rb_define_const(mErrors, "CROSSTARGET", INT2NUM(EZFS_CROSSTARGET));
  rb_define_const(mErrors, "ZONED", INT2NUM(EZFS_ZONED));
  rb_define_const(mErrors, "MOUNTFAILED", INT2NUM(EZFS_MOUNTFAILED));
  rb_define_const(mErrors, "UMOUNTFAILED", INT2NUM(EZFS_UMOUNTFAILED));
  rb_define_const(mErrors, "UNSHARENFSFAILED", INT2NUM(EZFS_UNSHARENFSFAILED));
  rb_define_const(mErrors, "SHARENFSFAILED", INT2NUM(EZFS_SHARENFSFAILED));
  rb_define_const(mErrors, "DEVLINKS", INT2NUM(EZFS_DEVLINKS));
  rb_define_const(mErrors, "PERM", INT2NUM(EZFS_PERM));
  rb_define_const(mErrors, "NOSPC", INT2NUM(EZFS_NOSPC));
  rb_define_const(mErrors, "IO", INT2NUM(EZFS_IO));
  rb_define_const(mErrors, "INTR", INT2NUM(EZFS_INTR));
  rb_define_const(mErrors, "ISSPARE", INT2NUM(EZFS_ISSPARE));
  rb_define_const(mErrors, "INVALCONFIG", INT2NUM(EZFS_INVALCONFIG));
  rb_define_const(mErrors, "RECURSIVE", INT2NUM(EZFS_RECURSIVE));
  rb_define_const(mErrors, "NOHISTORY", INT2NUM(EZFS_NOHISTORY));
  rb_define_const(mErrors, "UNSHAREISCSIFAILED", INT2NUM(EZFS_UNSHAREISCSIFAILED));
  rb_define_const(mErrors, "SHAREISCSIFAILED", INT2NUM(EZFS_SHAREISCSIFAILED));
  rb_define_const(mErrors, "POOLPROPS", INT2NUM(EZFS_POOLPROPS));
  rb_define_const(mErrors, "POOL_NOTSUP", INT2NUM(EZFS_POOL_NOTSUP));
  rb_define_const(mErrors, "POOL_INVALARG", INT2NUM(EZFS_POOL_INVALARG));
  rb_define_const(mErrors, "NAMETOOLONG", INT2NUM(EZFS_NAMETOOLONG));
  rb_define_const(mErrors, "UNKNOWN", INT2NUM(EZFS_UNKNOWN));
  
  /* Pool health status codes. */
  rb_define_const(mHealthStatus, "CORRUPT_CACHE", INT2NUM(ZPOOL_STATUS_CORRUPT_CACHE));
  rb_define_const(mHealthStatus, "MISSING_DEV_R", INT2NUM(ZPOOL_STATUS_MISSING_DEV_R));
  rb_define_const(mHealthStatus, "MISSING_DEV_NR", INT2NUM(ZPOOL_STATUS_MISSING_DEV_NR));
  rb_define_const(mHealthStatus, "CORRUPT_LABEL_R", INT2NUM(ZPOOL_STATUS_CORRUPT_LABEL_R));
  rb_define_const(mHealthStatus, "CORRUPT_LABEL_NR", INT2NUM(ZPOOL_STATUS_CORRUPT_LABEL_NR));
  rb_define_const(mHealthStatus, "BAD_GUID_SUM", INT2NUM(ZPOOL_STATUS_BAD_GUID_SUM));
  rb_define_const(mHealthStatus, "CORRUPT_POOL", INT2NUM(ZPOOL_STATUS_CORRUPT_POOL));
  rb_define_const(mHealthStatus, "CORRUPT_DATA", INT2NUM(ZPOOL_STATUS_CORRUPT_DATA));
  rb_define_const(mHealthStatus, "FAILING_DEV", INT2NUM(ZPOOL_STATUS_FAILING_DEV));
  rb_define_const(mHealthStatus, "VERSION_NEWER", INT2NUM(ZPOOL_STATUS_VERSION_NEWER));
  rb_define_const(mHealthStatus, "HOSTID_MISMATCH", INT2NUM(ZPOOL_STATUS_HOSTID_MISMATCH));
  rb_define_const(mHealthStatus, "VERSION_OLDER", INT2NUM(ZPOOL_STATUS_VERSION_OLDER));
  rb_define_const(mHealthStatus, "RESILVERING", INT2NUM(ZPOOL_STATUS_RESILVERING));
  rb_define_const(mHealthStatus, "OFFLINE_DEV", INT2NUM(ZPOOL_STATUS_OFFLINE_DEV));
  rb_define_const(mHealthStatus, "OK", INT2NUM(ZPOOL_STATUS_OK));
  
  /* Pool state codes */
  rb_define_const(mPoolState, "ACTIVE", INT2NUM(POOL_STATE_ACTIVE));
  rb_define_const(mPoolState, "EXPORTED", INT2NUM(POOL_STATE_EXPORTED));
  rb_define_const(mPoolState, "DESTROYED", INT2NUM(POOL_STATE_DESTROYED));
  rb_define_const(mPoolState, "SPARE", INT2NUM(POOL_STATE_SPARE));
  rb_define_const(mPoolState, "UNINITIALIZED", INT2NUM(POOL_STATE_UNINITIALIZED));
  rb_define_const(mPoolState, "UNAVAIL", INT2NUM(POOL_STATE_UNAVAIL));
  rb_define_const(mPoolState, "POTENTIALLY_ACTIVE", INT2NUM(POOL_STATE_POTENTIALLY_ACTIVE));
}

void Init_libzfs()
{
  VALUE cLibZfs = rb_define_class("LibZfs", rb_cObject);
  VALUE cZpool = rb_define_class("Zpool", rb_cObject);
  VALUE cZFS = rb_define_class("ZFS", rb_cObject);

  Init_libzfs_consts();
  
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
  rb_define_method(cZpool, "destroy!", my_zpool_destroy, 0);
  
  rb_define_singleton_method(cZpool, "each", my_zpool_iter, 1);
  
  rb_define_singleton_method(cZFS, "new", my_zfs_new, -1);
  rb_define_method(cZFS, "libzfs_handle", my_zfs_get_handle, 0);
  rb_define_method(cZFS, "is_shared?", my_zfs_is_shared, 0);
  rb_define_method(cZFS, "share!", my_zfs_share, 0);
  rb_define_method(cZFS, "unshare!", my_zfs_unshare, 0);
}
