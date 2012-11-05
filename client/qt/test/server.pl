use IO::Socket;
 
$port = 4243;
$SIG{'INT'} = 'cleanup';
 
create_listener();
 
sub create_listener { 
    my $sock = new IO::Socket::INET (
	LocalHost => 'localhost',
	LocalPort => '4243',
	Proto => 'tcp',
	Listen => 1,
	Reuse => 1,
	);

    die "Could not create socket: $!\n" unless $sock;

    while (1)
    {
	my $new_sock = $sock->accept();
        print "Creating child process to serve client\n";
        my $child;
 
        if ( ( $child = fork() ) == 0 ) {
 
	    my $send;
	    my $send = "{\"AddLog\": \"Add peers Racoon and Bob\"}";
	    send_request($new_sock, $send);
	    sleep 1;
	    $send = "[{\"AddContact\": {\"Name\":\"Racoon\", \"Key\": \"Public Key 4\"}}, {\"AddContact\":{\"Name\":\"Bob\", \"Key\": \"Public Key 1\"}}]\n";
	    send_request($new_sock, $send);
	    sleep 1;
	    my $send = "{\"AddLog\": \"Delete the peer Bob\"}";
	    send_request($new_sock, $send);
	    sleep 1;
	    my $send = "{\"DeleteContact\": \"Bob\"}";
	    send_request($new_sock, $send);
	    sleep 1;
 	    my $send = "{\"AddLog\": \"Edit Racoon to become Fox with key Fire\"}\n";
 	    send_request($new_sock, $send);
	    sleep 1;
 	    my $send = "{\"EditContact\": {\"Name\": \"Fox\", \"Old\": \"Racoon\", \"Key\": \"Fire\"}}\n";
 	    send_request($new_sock, $send);
            chomp($request = <$new_sock>);
 
            while ( not( $request =~ /quit/i ) ) {
 
                if (length($request) > 1){
		    print "Command received : $request\n";
                }
 
                chomp( $request = <$new_sock> );
 
            }
            close($new_sock)        or die "close error: $!";
            close($sock) or die "Erreur close : $!";
 
            exit(0);
        }
 
 
    }
 
}
 
sub send_request {
 
    $sock = $_[0];
    $res = $_[1];
    print $sock $res;
}
 
 
sub cleanup {
    print "\n\nCaught Interrupt (^C), Aborting\n";
    close(SOCK);
    close(SOCK_SERVER);
    exit(1);
}
 
1;
