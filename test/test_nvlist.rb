require 'rubygems'
require 'test/spec'
require 'nvlist'

context "A C Extension that hopefully initializes" do
  specify "I can actually initialize it" do
    n = NVList.new
  end
end