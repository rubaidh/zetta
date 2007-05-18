require 'mkmf'

pkg_name = File.basename(File.dirname(File.expand_path(__FILE__)))

dir_config(pkg_name)
have_library("c", "main")

# Check for prerequisite ZFS header and library.
have_library('zfs', 'zpool_create') || failed_prereqs = true
have_header('libzfs.h') || failed_prereqs = true

create_makefile(pkg_name) unless failed_prereqs
