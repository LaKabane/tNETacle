# Copyright (c) 2013 Antoine Marandon <ntnrmrndn AT gmail DOT com>

# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.

# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


# -*- coding: utf-8 -*-
puts 'Initializing...'
require '../../../sub/libtclt/wrapper/ruby/rubtclt.rb'
require 'socket'
require 'thread'
require 'highline/import'

#init conf
require 'conf.rb'

def validate
  validate = ask 'Is this correct ? [Y/N]'
  if validate == 'Y' || validate == 'y'
    return true
  else
    return false
  end
end
#init Rtclt
Rtclt::init
Rtclt::set_callback_add_peer do |peer|
  puts 'Added new peer: ' + peer.name
  puts ' - Key of new peer is: ' + peer.key
  puts ' - Ip of new peer is: ' + peer.ip
end
Rtclt::set_callback_del_peer do |peer|
  puts 'Deleted peer:' + peer.name
  puts ' - Key was:' + peer.key
  puts ' - Ip was:' + peer.ip
end
Rtclt::set_callback_add_log do |log_str|
  puts 'Received log - line start above -'
  puts log_str
  puts 'Log end here'
end
#init socket
puts 'Openning tcp socket to ' + Conf::instance.host + ':' + Conf::instance.port
socket = TCPSocket.open Conf::instance.host, Conf::instance.port
socket.sync = true

#network loop
Thread.new do
  if Conf::instance.debug
    puts 'Spawned new thread'
  end
  while  line = socket.gets
    if Conf::instance.debug
      puts line
    end
    Rtclt::parse line
  end
  puts 'Closing connection'
  s.close
end

#user loop
while 42
  stop = false
  choose  do |menu|
    menu.prompt = ''
    # => help
    menu.choice 'help' do
      puts 'Naaah... I won\'t help you'
    end
    # => Add contact
    menu.choice 'Add a contact' do
      puts '----- Add contact -----'
      ip = ask 'ip ?'
      name = ask 'Name ?'
      key = ask 'Key ?'
      puts 'Will add: name' + name + ' ip: ' + ip + ' key : ' + key
      if validate
        puts 'adding name' + name + ' ip: ' + ip + ' key : ' + key
        peer = Rtclt::Peer.new
        peer.name = name
        peer.ip = ip
        peer.key = key
        json = Rtclt::add_peer peer
        socket.send json, 0
      else
        puts 'aborting'
      end
    end
    # => Delete user
    menu.choice 'Delete user' do
      puts '----- Delete user -----'
      name = nil;
      key = nil;
      ip = nil;
      puts 'Delete by name ?'
      if validate
        name = ask 'Name ?'
      end
      puts 'Delete by ip ?'
      if validate
        ip = ask 'ip ?'
      end
      puts 'Delete by key ?'
      if validate
        key = ask 'Key ?'
      end
      if ip
        puts 'Delete by ip: ' + ip
        if validate
          puts 'Deleting'
        else
          puts 'aborting'
        end
      elsif name
        puts 'Delete by name: ' + name
        if validate
          puts 'Deleting'
        else
          puts 'aborting'
        end
      elsif key
        puts 'Delete by key: ' + key
        if validate
          puts 'Deleting'
        else
          puts 'aborting'
        end
      end
    end
    # => JSON cmd
    menu.choice 'JSON cmd' do
      puts '----- JSON Raw cmd -----'
      cmd = ask 'type json'
      puts 'will send: "' + cmd + '" is it correct ?'
    end
    # => exit
    menu.choice 'exit' do
      puts 'bye'
      stop = true
    end
  end
  if stop
    break
  end
end
  #terminate client
Rtclt::destroy
