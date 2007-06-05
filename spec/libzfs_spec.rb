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
    @pool_name = 'pool'
    @pool = Zpool.new(@pool_name, @z)
  end

  it "can successfully open the pool" do
    @pool.should_not be_nil
    @z.errno.should == 0
    @z.error_action.should. == ""
    @z.error_description.should == "no error"
  end

  # Yeah, I know, what am I doing testing something so basic?  Because this
  # is the first C extension I've written and I want to be sure I've done
  # that bit right!
  it "can't be opened if we don't supply the right arguments" do
    lambda do
      too_few_arguments_to_pool = Zpool.new(@pool_name)
    end.should raise_error(ArgumentError)

    lambda do
      too_many_arguments_to_pool = Zpool.new(@pool_name, @z, :another_arg)
    end.should raise_error(ArgumentError)
  end
  
  it "should be able to retrieve the libzfs handle" do
    h = @pool.libzfs_handle
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
  
  it "should allow us to retrieve the name of the pool" do
    @pool.name.should == @pool_name
  end

  it "should allow us to retrieve the guid" do
    # Unfortunately, in order to make it portable, there's not much more
    # we can say about it!
    @pool.guid.should > 0
  end
  
  it "should allow us to query space information on the pool" do
    # I think these are all invariant.
    @pool.space_total.should > 0
    @pool.space_used.should > 0
    @pool.space_total.should > @pool.space_used
  end
  
  it "should allow us to query the altroot, which will be nil" do
    @pool.root.should == nil
  end
  
  it "should give us the current pool status, which will be ACTIVE" do
    @pool.state.should == ZfsConsts::State::Pool::ACTIVE
  end
  
  it "should have the version of the pool, which should be something sane" do
    @pool.version.should > 0
    @pool.version.should <= ZfsConsts::VERSION
  end
end

describe "trying to access a non-existant pool" do
  before do
    @z = LibZfs.new
  end

  it "will, believe it or not, give you a handle back but with an error set" do
    pool = Zpool.new('nonexistentpool', @z)
    pool.should_not be_nil
    
    # But you'll get an error
    @z.errno.should == ZfsConsts::Errors::NOENT
    @z.error_action.should == "cannot open 'nonexistentpool'"
    @z.error_description.should == "no such pool"
  end
  
end

describe "All the constants in libzfs.h" do
  it "should make the filesystem types available" do
    ZfsConsts::Types::FILESYSTEM.should == 1
    ZfsConsts::Types::SNAPSHOT.should   == 2
    ZfsConsts::Types::VOLUME.should     == 4
    ZfsConsts::Types::POOL.should       == 8
    ZfsConsts::Types::ANY.should == ZfsConsts::Types::FILESYSTEM | ZfsConsts::Types::SNAPSHOT | ZfsConsts::Types::VOLUME
  end
  
  it "should make the error types available" do
    ZfsConsts::Errors::NOMEM.should               == 2000
    ZfsConsts::Errors::BADPROP.should             == 2001
    ZfsConsts::Errors::PROPREADONLY.should        == 2002
    ZfsConsts::Errors::PROPTYPE.should            == 2003
    ZfsConsts::Errors::PROPNONINHERIT.should      == 2004
    ZfsConsts::Errors::PROPSPACE.should           == 2005
    ZfsConsts::Errors::BADTYPE.should             == 2006
    ZfsConsts::Errors::BUSY.should                == 2007
    ZfsConsts::Errors::EXISTS.should              == 2008
    ZfsConsts::Errors::NOENT.should               == 2009
    ZfsConsts::Errors::BADSTREAM.should           == 2010
    ZfsConsts::Errors::DSREADONLY.should          == 2011
    ZfsConsts::Errors::VOLTOOBIG.should           == 2012
    ZfsConsts::Errors::VOLHASDATA.should          == 2013
    ZfsConsts::Errors::INVALIDNAME.should         == 2014
    ZfsConsts::Errors::BADRESTORE.should          == 2015
    ZfsConsts::Errors::BADBACKUP.should           == 2016
    ZfsConsts::Errors::BADTARGET.should           == 2017
    ZfsConsts::Errors::NODEVICE.should            == 2018
    ZfsConsts::Errors::BADDEV.should              == 2019
    ZfsConsts::Errors::NOREPLICAS.should          == 2020
    ZfsConsts::Errors::RESILVERING.should         == 2021
    ZfsConsts::Errors::BADVERSION.should          == 2022
    ZfsConsts::Errors::POOLUNAVAIL.should         == 2023
    ZfsConsts::Errors::DEVOVERFLOW.should         == 2024
    ZfsConsts::Errors::BADPATH.should             == 2025
    ZfsConsts::Errors::CROSSTARGET.should         == 2026
    ZfsConsts::Errors::ZONED.should               == 2027
    ZfsConsts::Errors::MOUNTFAILED.should         == 2028
    ZfsConsts::Errors::UMOUNTFAILED.should        == 2029
    ZfsConsts::Errors::UNSHARENFSFAILED.should    == 2030
    ZfsConsts::Errors::SHARENFSFAILED.should      == 2031
    ZfsConsts::Errors::DEVLINKS.should            == 2032
    ZfsConsts::Errors::PERM.should                == 2033
    ZfsConsts::Errors::NOSPC.should               == 2034
    ZfsConsts::Errors::IO.should                  == 2035
    ZfsConsts::Errors::INTR.should                == 2036
    ZfsConsts::Errors::ISSPARE.should             == 2037
    ZfsConsts::Errors::INVALCONFIG.should         == 2038
    ZfsConsts::Errors::RECURSIVE.should           == 2039
    ZfsConsts::Errors::NOHISTORY.should           == 2040
    ZfsConsts::Errors::UNSHAREISCSIFAILED.should  == 2041
    ZfsConsts::Errors::SHAREISCSIFAILED.should    == 2042
    ZfsConsts::Errors::POOLPROPS.should           == 2043
    ZfsConsts::Errors::POOL_NOTSUP.should         == 2044
    ZfsConsts::Errors::POOL_INVALARG.should       == 2045
    ZfsConsts::Errors::NAMETOOLONG.should         == 2046
    ZfsConsts::Errors::UNKNOWN.should             == 2047
  end
  
  it "should make the pool health status code available" do
    ZfsConsts::HealthStatus::CORRUPT_CACHE.should     == 0
    ZfsConsts::HealthStatus::MISSING_DEV_R.should     == 1
    ZfsConsts::HealthStatus::MISSING_DEV_NR.should    == 2
    ZfsConsts::HealthStatus::CORRUPT_LABEL_R.should   == 3
    ZfsConsts::HealthStatus::CORRUPT_LABEL_NR.should  == 4
    ZfsConsts::HealthStatus::BAD_GUID_SUM.should      == 5
    ZfsConsts::HealthStatus::CORRUPT_POOL.should      == 6
    ZfsConsts::HealthStatus::CORRUPT_DATA.should      == 7
    ZfsConsts::HealthStatus::FAILING_DEV.should       == 8
    ZfsConsts::HealthStatus::VERSION_NEWER.should     == 9
    ZfsConsts::HealthStatus::HOSTID_MISMATCH.should   == 10
    ZfsConsts::HealthStatus::VERSION_OLDER.should     == 11
    ZfsConsts::HealthStatus::RESILVERING.should       == 12
    ZfsConsts::HealthStatus::OFFLINE_DEV.should       == 13
    ZfsConsts::HealthStatus::OK.should                == 14
  end
  
  it "should make the pool states available" do
    ZfsConsts::State::Pool::ACTIVE.should == 0
    ZfsConsts::State::Pool::EXPORTED.should == 1
    ZfsConsts::State::Pool::DESTROYED.should == 2
    ZfsConsts::State::Pool::SPARE.should == 3
    ZfsConsts::State::Pool::UNINITIALIZED.should == 4
    ZfsConsts::State::Pool::UNAVAIL.should == 5
    ZfsConsts::State::Pool::POTENTIALLY_ACTIVE.should == 6
  end
end

describe "Given a ZFS filesystem called 'pool/mathie'" do
  before do
    @z = LibZfs.new
    @fs = ZFS.new('pool/mathie', @z, ZfsConsts::Types::ANY)
  end

  it "we can open up the filesystem" do
    @fs.should_not be_nil
    @z.errno.should == 0
    @z.error_action.should == ""
    @z.error_description.should == "no error"
  end

end

describe "Given an existing ZFS filesystem called 'pool/shared' which has the sharenfs property set to true" do
  before do
    @z = LibZfs.new
    @fs = ZFS.new('pool/shared', @z, ZfsConsts::Types::ANY)
  end

  it "should be shareable and unshareable" do
    @fs.is_shared?.should == true
    lambda { @fs.unshare!.should == 0 }.should_not raise_error
    @fs.is_shared?.should == false
    lambda { @fs.share!.should == 0 }.should_not raise_error
    @fs.is_shared?.should == true
  end
end

describe "Given an existing ZFS filesystem called 'pool/unshared'" do
  before do
    @z = LibZfs.new
    @fs = ZFS.new('pool/unshared', @z, ZfsConsts::Types::ANY)
  end

  it "should ignore any attempts to share and unshare it because sharenfs is off" do
    @fs.is_shared?.should == false
    lambda { @fs.unshare!.should == 0 }.should_not raise_error
    @fs.is_shared?.should == false
    lambda { @fs.share!.should == 0 }.should_not raise_error
    @fs.is_shared?.should == false
  end
end
describe "Given a non-existant filesystem 'pool/nonexistent'" do
  before do
    @z = LibZfs.new
  end
  
  it "Until I fix it, we can create the object, but @z notes the error." do
    fs = ZFS.new('pool/nonexistent', @z, ZfsConsts::Types::ANY)
    fs.should_not be_nil
    @z.errno.should == ZfsConsts::Errors::NOENT
    @z.error_action.should == "cannot open 'pool/nonexistent'"
    @z.error_description.should == "dataset does not exist"
  end
end