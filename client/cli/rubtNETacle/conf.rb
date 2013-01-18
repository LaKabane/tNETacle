require 'singleton'

class Conf
  include Singleton
  attr_accessor :host, :port, :debug
  def initialize
    @host = '127.0.0.1';
    @port = '4243';
    @debug = true;
  end
end
