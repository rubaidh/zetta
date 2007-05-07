require 'mkmf'

pkg_name = File.basename(File.dirname(File.expand_path(__FILE__)))

dir_config(pkg_name)
have_library("c", "main")

create_makefile(pkg_name)
