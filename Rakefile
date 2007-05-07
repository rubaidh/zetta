require 'rubygems'
require 'rake'
require 'rake/clean'
require 'rake/gempackagetask'
require 'rake/testtask'
require 'rake/rdoctask'

desc "Compiles and tests the build"
task :default => [ :compile, :test ]

MAJOR_VERSION = "0.1"

SPEC = Gem::Specification.new do |s|
  # Stuff I might want to tweak.
  s.summary = "A Ruby interface to manage ZFS."

  # Calculate version from the +MAJOR_VERSION+ constant above, and the current
  # svn revision, if applicable.
  revision = `svn info`[/Revision: (\d+)/, 1] rescue nil
  s.version = MAJOR_VERSION + (revision ? ".#{revision}" : "")

  # Usual constants
  s.name = File.split(File.dirname(File.expand_path(__FILE__)))[-1]
  s.author = "Graeme Mathieson"
  s.email = "mathie@rubaidh.com"
  s.homepage = "http://www.rubaidh.com/projects/#{s.name}"
  s.platform = Gem::Platform::RUBY # FIXME: Maybe this should be Solaris?
  s.description = s.summary
  s.files = %w(CHANGELOG Rakefile README) +
    FileList["{bin,doc,lib,test}/**/*"].exclude("rdoc").to_a +
    FileList["ext/**/*.{h,c}"].to_a
  s.require_path = "lib"
  s.extensions = FileList["ext/**/extconf.rb"].to_a
  s.bindir = 'bin'
  s.test_files = FileList["test/test_*.rb"].to_a

  # Documentation
  s.has_rdoc = true
  s.extra_rdoc_files = ['README', 'CHANGELOG']
end

Rake::TestTask.new do |t|
  t.libs << 'test'
  t.test_files = SPEC.test_files
  t.verbose = true
end

Rake::RDocTask.new do |rdoc|
  rdoc.rdoc_dir = 'doc/rdoc'
  rdoc.options += ['--line-numbers', '--inline-sources']
  rdoc.main = 'README'
  rdoc.rdoc_files.add [SPEC.extra_rdoc_files, 'lib/**/*.rb'].flatten
end

Rake::GemPackageTask.new(SPEC) do |p|
  p.need_tar_bz2  = true
  p.need_zip      = true
end
