require '../../../sub/libtclt/wrapper/ruby/rubtclt.rb'

Rtclt::init
Rtclt::set_callback_add_peer do |peer|
  puts peer.name
  puts peer.key
  puts peer.ip
end
Rtclt::parse '{"AddContact":{"Name":"contact1","Key":"key1","Ip":"10.0.0.1"}}'
Rtclt::destroy
