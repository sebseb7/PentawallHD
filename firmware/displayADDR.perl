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


while(1)
{
	my $return=$port->write(chr(0x66).chr(255));
	usleep(100000);
}
sleep(1);
warn $port->read(1);
