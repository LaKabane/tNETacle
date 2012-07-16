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
	    my $send = "{\"AddLog\": { \"Log\" : \"Add peers Racoon and Bob\"}}";
	    send_request($new_sock, $send);
	    $send = "{
    \"AddContact\": {
	\"Racoon\": {
	    \"Key\": \"Public Key 4\"
	},
	\"Bob\": {
	    \"Key\": \"Public Key 1\"
	}
    }
}";
	    send_request($new_sock, $send);
	    my $send = "{\"AddLog\": { \"Log\" : \"Delete the peer Bob\"}}";
	    send_request($new_sock, $send);
	    my $send = "{\"DeleteContact\": { \"Bob\" : {\"Name\": \"Bob\"}}}";
	    send_request($new_sock, $send);
	    my $send = "{\"AddLog\": { \"Log\" : \"Edit Racoon to become Fox with key Fire\"}}";
	    send_request($new_sock, $send);
	    my $send = "{\"EditContact\": { \"Racoon\" : {\"Name\": \"Fox\", \"Old\": \"Racoon\", \"Key\": \"Fire\"}}}";
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
    sleep 1;
}
 
 
sub cleanup {
    print "\n\nCaught Interrupt (^C), Aborting\n";
    close(SOCK);
    close(SOCK_SERVER);
    exit(1);
}
 
1;
