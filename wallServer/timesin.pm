#!/usr/bin/perl



package timesin;


use strict;
use Time::HiRes qw(time);

my $pi  = 3.141592653589793238462643;


sub timesin($$$)
{
	my $from = shift;
	my $to = shift;
	my $duration = shift;

	my $span = ($to-$from)/2;
	my $center = ($to+$from)/2;
	
	return (sin(time*2*$pi/$duration*.6)*$span)+$center;
	

}


1;
