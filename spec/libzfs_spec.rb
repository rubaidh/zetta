$:.unshift File.join(File.dirname(__FILE__), "..", "lib")

require 'rubygems'
require 'spec'

# Library under test
require 'libzfs'

describe LibZfs, " freshly created" do
  
  before do
    @z = LibZfs.new
  end

  it { @z.should_not equal(nil) }
  
  it "should be able to retrieve an error status of 0." do
    @z.errno.should == 0
    @z.error_action.should == ""
    @z.error_description.should == "no error"
  end

  it "should be able to toggle the print_on_error thing without anything blowing up" do
    lambda { @z.print_on_error(true) }.should_not raise_error
    lambda { @z.print_on_error(false) }.should_not raise_error
  end

end