#!/usr/bin/perl

use Device::SerialPort;
use Time::HiRes qw(usleep);

my $port = Device::SerialPort->new("/dev/cu.usbserial-A2002QDi");
$port->databits(8);
$port->baudrate(230400);
$port->parity("none");
$port->stopbits(1);

my $return=$port->write(chr(102).chr(3)."\n");
sleep(1);
warn $port->read(1);
