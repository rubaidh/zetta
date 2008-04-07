require "test/unit"

$:.unshift File.dirname(__FILE__) + "/../ext/libzfs"
require "libzfs.so"

class TestLibzfsExtn < Test::Unit::TestCase
  def test_truth
    assert true
  end
end