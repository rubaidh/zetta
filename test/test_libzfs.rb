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

  specify "We can switch on and off the print_on_error thing without anything blowing up" do
    z = LibZfs.new
    lambda { z.print_on_error(true) }.should.not.raise
    lambda { z.print_on_error(false) }.should.not.raise
  end
end

# FIXME: This currently requires that a pool is already created and available
# on the system, and that it's called "pool".  To create the pool, do
# something along the lines of:
# 
#   sudo mkfile 100m /path/to/{a,b,c,d}
#   sudo zpool create pool raidz2 /path/to/{a,b,c,d}
context "Given a precreated pool on the filesystem called 'pool'" do
  
  setup do
    @z = LibZfs.new
  end

  specify "We can successfully open the pool" do
    pool = Zpool.new('pool', @z)
    pool.should.not.be nil
    @z.errno.should.be 0
    @z.error_action.should.equal ""
    @z.error_description.should.equal "no error"
  end
  
  # Yeah, I know, what am I doing testing something so basic?  Because this
  # is the first C extension I've written and I want to be sure I've done
  # that bit right!
  specify "We can't open a pool if we don't supply the right arguments" do
    lambda do
      pool = Zpool.new('pool')
    end.should.raise ArgumentError

    lambda do
      pool = Zpool.new('pool', @z, :another_arg)
    end.should.raise ArgumentError
  end
  
  specify "Belive it or not, we can create non-existent pools" do
    pool = Zpool.new('nonexistentpool', @z)
    pool.should.not.be nil
    
    # But you'll get an error
    @z.errno.should.not.be 0
    @z.error_action.should.equal "cannot open 'nonexistentpool'" # => Dunno yet!
    @z.error_description.should.equal "no such pool" # => Dunno yet!
  end
  
  specify "We can get the libzfs handle back from an open pool" do
    pool = Zpool.new('pool', @z)
    h = pool.libzfs_handle
    h.should.not.be.nil
    h.errno.should.be 0
    h.error_action.should.equal ""
    h.error_description.should.equal "no error"
    
    # FIXME: One or both of the following should be true:
    # h.should.equal @z
    # h.should.be @z
  end
  
  # specify "We can pass in something other than a string to the pool open" do
  #   pool = Zpool.new(:pool, @z)
  #   pool.should.not.be nil
  #   @z.errno.should.be 0
  #   @z.error_action.should.equal ""
  #   @z.error_description.should.equal "no error"
  # end
  
end