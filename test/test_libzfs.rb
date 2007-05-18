require 'rubygems'
require 'test/spec'
require 'libzfs'

context "With the low level ZFS handle interface" do
  specify "I can actually initialize it" do
    z = LibZfs.new
    z.should.not.be nil
  end
  
  specify "We can retrieve the default error information (no error)" do
    z = LibZfs.new
    z.errno.should.be 0
    z.error_action.should.equal ""
    z.error_description.should.equal "no error"
  end
end