#!/usr/bin/perl

package g3d2;

use strict;
use IO::Socket;
use Time::HiRes qw(usleep);

my $socket;
my $window = 3;

sub init()
{
	$socket = IO::Socket::INET->new(PeerAddr => 'g3d2',
									PeerPort => 1339,
									Proto    => "tcp",
									Type     => SOCK_STREAM)     or die "Couldn't connect : $@\n";
}

sub record()
{
	print $socket "05\r\n";
}


sub setPixel($$$)
{
	my $x = shift;
	my $y = shift;
	my $green = shift;


	print $socket '02'.sprintf("%02x",$x).sprintf("%02x",$y).sprintf("%1x",$green)."\r\n";
}

sub setAll($)
{
	my $green = shift;


	print $socket '02'.sprintf("%1x",$green)."\r\n";
}

sub setFrame($)
{
	my $frame=shift;

	my $ppp = 72;
	
	print $socket '03';
		
	for(0..((2304/$ppp)-1))
	{

			my $packet = $_;
			
			my $data;
			for(0..($ppp-1))
			{
				$data.=sprintf("%1x", $frame->[$packet*$ppp+$_]);
			}
			
			print $socket $data;

			#implement flowcontrol	
			usleep(700);

	}
	print $socket "\r\n";


	if(--$window > 0)
	{
	my $data;

	do
	{
		$socket->recv($data,1, 0);
	}until(ord($data) == 10);
	}
	

}


sub binFrame($)
{
	my $frame=shift;

	print $socket $frame;
#	print $socket2 $frame;

	print $socket "\r\n";
#	print $socket2 "\r\n";
	
#	warn <$socket>;
}

sub setLevel($)
{
	my $level=shift;

	print $socket '04'.sprintf("%02x", $level);
#	print $socket2 '04'.sprintf("%02x", $level);
		
	print $socket "\r\n";
#	print $socket2 "\r\n";

#	warn <$socket>;
}

sub readline()
{
#	my $liney = <$socket2>;
	my $line = <$socket>;
	return $line;
}


1;


