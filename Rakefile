require 'rubygems'
require 'rake'
require 'rake/clean'
require 'rake/gempackagetask'
require 'rake/rdoctask'

require 'spec/rake/spectask'

desc "Compiles and tests the build"
task :default => [ :compile, :spec ]

MAJOR_VERSION = "0.1"

CLEAN.include ["ext/**/*.bundle", "lib/*.bundle", "ext/**/{Makefile,mkmf.log,*.o}", "*.gem", 'pkg']

Gem::manage_gems
SPEC = Gem::Specification.new do |s|
  # Stuff I might want to tweak.
  s.summary = "A Ruby interface to manage ZFS."

  # Calculate version from the +MAJOR_VERSION+ constant above, and the current
  # svn revision, if applicable.
  revision = `svn info`[/Revision: (\d+)/, 1] rescue nil
  s.version = MAJOR_VERSION + (revision ? ".#{revision}" : "")

  # Usual constants
  s.name = File.basename(File.dirname(File.expand_path(__FILE__)))
  s.author = "Graeme Mathieson"
  s.email = "mathie@rubaidh.com"
  s.homepage = "http://rubaidh.lighthouseapp.com/projects/2308/home"
  s.platform = Gem::Platform::RUBY # FIXME: Maybe this should be Solaris?
  s.description = s.summary
  s.files = (%w(CHANGELOG Rakefile README) +
      FileList["{bin,doc,lib,spec}/**/*"].to_a +
      FileList["ext/**/*.{h,c}"].to_a).delete_if do |f|
    f =~ /^\._/ ||
    f =~ /doc\/(rdoc|coverage)/ ||
    f =~ /\.(so|bundle)$/
  end
  s.require_path = "lib"
  s.extensions = FileList["ext/**/extconf.rb"].to_a
  s.bindir = 'bin'
  s.test_files = FileList["spec/*.rb"].to_a

  # Documentation
  s.has_rdoc = true
  s.extra_rdoc_files = ['README', 'CHANGELOG']
end

Spec::Rake::SpecTask.new do |spec|
  spec.spec_files = FileList['spec/**/*_spec.rb']
  spec.rcov = true
  spec.rcov_dir = "doc/coverage"
end

Rake::RDocTask.new do |rdoc|
  rdoc.rdoc_dir = 'doc/rdoc'
  rdoc.options += ['--line-numbers', '--inline-source']
  rdoc.main = 'README'
  rdoc.rdoc_files.add [SPEC.extra_rdoc_files, 'lib/**/*.rb'].flatten
end

Rake::GemPackageTask.new(SPEC) do |p|
end

extensions = Dir.glob('ext/*').map { |f| File.basename(f).to_sym }

desc "Compile the Ruby extensions"
task :compile => extensions do
  if Dir.glob("lib/{#{extensions.map.join(',')}}.*").length != extensions.length
    STDERR.puts "One or more of the extensions failed to build.  But I didn't tell you sooner!"
    exit(1)
  end
end

extensions.each do |extension|
  ext = "ext/#{extension}"
  ext_so = "#{ext}/#{extension}.#{Config::CONFIG['DLEXT']}"
  ext_files = FileList[
    "#{ext}/*.c",
    "#{ext}/*.h",
    "#{ext}/extconf.rb",
    "#{ext}/Makefile",
    "lib"
  ]

  desc "Builds just the #{extension} extension."
  task extension => [ "#{ext}/Makefile", ext_so ]

  file "#{ext}/Makefile" => [ "#{ext}/extconf.rb" ] do
    Dir.chdir(ext) do
      ruby "extconf.rb"
    end
  end
  
  file ext_so => ext_files do
    Dir.chdir(ext) do
      sh 'make'
    end
    cp ext_so, "lib"
  end
end