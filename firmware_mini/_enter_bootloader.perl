#!/usr/bin/perl

use Device::SerialPort;
use Time::HiRes qw(usleep);

my $hostname = `hostname`;
chomp $hostname;

my $port = "/dev/cu.usbserial-A100DDXL";
if($hostname eq 'lennyvm')
{
	$port = '/dev/ttyUSB0';
}

my $port = Device::SerialPort->new($port);
$port->databits(8);
$port->baudrate(230400);
$port->parity("none");
$port->stopbits(1);

my $return=$port->write(chr(102)."\n");
warn $port->read(1);

