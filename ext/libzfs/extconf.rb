require 'mkmf'

dir_config("libzfs")

have_library('zfs', 'zpool_create') || failed_prerequisites = true
have_header('libzfs.h') || failed_prerequisites = true

if failed_prerequisites
  exit 1
else
  create_makefile("libzfs")
end
