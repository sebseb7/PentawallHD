#!/usr/bin/perl

use Device::SerialPort;
use Time::HiRes qw(usleep);

my $hostname = `hostname`;
chomp $hostname;

my $port = "/dev/cu.usbserial-A100DEF2";
if($hostname eq 'lennyvm')
{
	$port = '/dev/ttyUSB0';
}

my $port = Device::SerialPort->new($port);
$port->databits(8);
$port->baudrate(500000);
$port->parity("none");
$port->stopbits(1);

my $return=$port->write(chr(102).esc(chr($ARGV[0]))."\n");
warn $port->read(1);

sub esc($)
{
    my $data = shift;
    
    
	$data =~ s/e/\x65\x3/go;
	$data =~ s/\x23/\x65\x1/go;
	$data =~ s/B/\x65\x2/go;
	$data =~ s/f/\x65\x4/go;
                                                            
	return $data;
}
