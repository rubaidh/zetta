$:.unshift File.join(File.dirname(__FILE__), "..", "lib")

require 'rubygems'
require 'spec'

# Library under test
require 'nvlist'

describe "A C Extension that hopefully initializes" do
  it "can actually be initialized" do
    n = NVList.new
    n.should_not be_nil
  end
end