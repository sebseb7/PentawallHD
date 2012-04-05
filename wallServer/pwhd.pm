#!/usr/bin/perl

package pwhd;

use strict;
use IO::Socket;
use Time::HiRes qw(usleep);
use IO::Socket::INET6;

my $socket;
my $window = 3;

sub init()
{
	$socket = IO::Socket::INET6->new(PeerAddr => '2001:6f8:1194:c3d2:223:dfff:fe7e:c80a',
									PeerPort => 1340,
									Proto    => "tcp",
									Type     => SOCK_STREAM)     or die "Couldn't connect : $@\n";
}

sub record()
{
	print $socket "05\r\n";
}


sub setPixel($$$$$)
{
	my $x = shift;
	my $y = shift;
	my $red = shift;
	my $green = shift;
	my $blue = shift;


	print $socket '02'.sprintf("%02x",$x).sprintf("%02x",$y).sprintf("%2x",$red).sprintf("%2x",$green).sprintf("%2x",$blue)."\r\n";
}

sub setAll($)
{
	my $red = shift;
	my $green = shift;
	my $blue = shift;

	print $socket '02ffff'.sprintf("%2x",$red),sprintf("%2x",$green).sprintf("%2x",$blue)."\r\n";
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


