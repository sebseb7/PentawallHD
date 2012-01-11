#!/usr/bin/perl

use Device::SerialPort;
use Time::HiRes qw(usleep);

my $port = Device::SerialPort->new("/dev/cu.usbserial-A100DDXL");
$port->databits(8);
$port->baudrate(230400);
$port->parity("none");
$port->stopbits(1);

while(1)
{
	my $return=$port->write(chr(66).esc(chr(00).chr(255).chr(0).chr(0)));
	usleep(50000);
	my $return=$port->write(chr(66).esc(chr(00).chr(00).chr(00).chr(0)));
	usleep(50000);
	my $return=$port->write(chr(66).esc(chr(00).chr(00).chr(00).chr(255)));
	usleep(50000);
	my $return=$port->write(chr(66).esc(chr(00).chr(00).chr(00).chr(0)));
	usleep(50000);
}
sleep(1);
warn $port->read(1);



sub esc($)
{
    my $data = shift;
    
    
	$data =~ s/\x65/\x65\x3/go;
	$data =~ s/\x67/\x65\x1/go;
	$data =~ s/\x68/\x65\x2/go;
	$data =~ s/\x66/\x65\x4/go;
                                                            
	return $data;
}
