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

# FIXME: This currently requires that a pool is already created and available
# on the system, and that it's called "pool".  To create the pool, do
# something along the lines of:
# 
#   sudo mkfile 100m /path/to/{a,b,c,d}
#   sudo zpool create pool raidz2 /path/to/{a,b,c,d}
describe Zpool, "given a precreated pool on the filesystem called 'pool'" do
  
  before do
    @z = LibZfs.new
  end

  it "can successfully open the pool" do
    pool = Zpool.new('pool', @z)
    pool.should_not be_nil
    @z.errno.should == 0
    @z.error_action.should. == ""
    @z.error_description.should == "no error"
  end

  # Yeah, I know, what am I doing testing something so basic?  Because this
  # is the first C extension I've written and I want to be sure I've done
  # that bit right!
  it "can't be opened if we don't supply the right arguments" do
    lambda do
      pool = Zpool.new('pool')
    end.should raise_error(ArgumentError)

    lambda do
      pool = Zpool.new('pool', @z, :another_arg)
    end.should raise_error(ArgumentError)
  end
  
  it "will, believe it or not, give you a handle back to a non-existent pool" do
    pool = Zpool.new('nonexistentpool', @z)
    pool.should_not be_nil
    
    # But you'll get an error
    @z.errno.should_not == 0
    @z.error_action.should == "cannot open 'nonexistentpool'"
    @z.error_description.should == "no such pool"
  end
  
  it "should be able to retrieve the libzfs handle" do
    pool = Zpool.new('pool', @z)
    h = pool.libzfs_handle
    h.should_not be_nil
    h.errno.should == 0
    h.error_action.should == ""
    h.error_description.should == "no error"
    
    # FIXME: One or both of the following should be true:
    # h.should == @z
    # h.should == @z
  end
  
  # FIXME: Right now we can't use symbols for pool names yet.
  # it "can pass in something other than a string to the pool open" do
  #   pool = Zpool.new(:pool, @z)
  #   pool.should.not.be nil
  #   @z.errno.should == 0
  #   @z.error_action.should == ""
  #   @z.error_description.should == "no error"
  # end
end

describe "All the constants in libzfs.h" do
  it "should make the filesystem types available" do
    ZfsConsts::TYPE_FILESYSTEM.should == 1
    ZfsConsts::TYPE_SNAPSHOT.should   == 2
    ZfsConsts::TYPE_VOLUME.should     == 4
    ZfsConsts::TYPE_POOL.should       == 8
    ZfsConsts::TYPE_ANY.should == ZfsConsts::TYPE_FILESYSTEM | ZfsConsts::TYPE_SNAPSHOT | ZfsConsts::TYPE_VOLUME
  end
end

describe "Given a ZFS filesystem called 'pool/mathie'" do
  before do
    @z = LibZfs.new
  end

  it "we can open up the filesystem" do
    fs = ZFS.new('pool/mathie', @z, ZfsConsts::TYPE_ANY)
    fs.should_not be_nil
    @z.errno.should == 0
    @z.error_action.should == ""
    @z.error_description.should == "no error"
  end
end

context "Given a non-existant filesystem 'pool/nonexistent'" do
  before do
    @z = LibZfs.new
  end
  
  it "Until I fix it, we can create the object, but @z notes the error." do
    fs = ZFS.new('pool/nonexistent', @z, ZfsConsts::TYPE_ANY)
    fs.should_not be_nil
    @z.errno.should == 2009 # FIXME: Magic numbers!
    @z.error_action.should == "cannot open 'pool/nonexistent'"
    @z.error_description.should == "dataset does not exist"
  end
end