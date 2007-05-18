require 'mkmf'

pkg_name = File.basename(File.dirname(File.expand_path(__FILE__)))

dir_config(pkg_name)
have_library("c", "main")

have_library('nvpair', 'nvlist_alloc') || failed_prereqs = true
have_header('libnvpair.h') || failed_prereqs = true

create_makefile(pkg_name) unless failed_prereqs
