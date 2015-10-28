#!/usr/bin/perl -I/Users/seb/gitrepos/PentawallHD/wallServer

use pwhd;
use ceilingLight;
use strict;
use Time::HiRes qw(usleep time);
use List::Util qw(shuffle);


pwhd::init();
pwhd::readline();

pwhd::setLevel(0);
pwhd::readline();

ceilingLight::init();
ceilingLight::readline();

ceilingLight::setLevel(0);
ceilingLight::readline();


my $PATH = $ENV{'HOME'}."/Sites/wallRecords";
$0 = 'idleloop';
my $event = '';
while(1)
{
	eval
	{


		my @files;
		
		opendir(my $dh, $PATH) || die "can't opendir: $!";
		while(my $file  = readdir($dh)) {
			next if $file !~  /\.rec$/;
			push @files,$file;
		}
		closedir $dh;
		@files = shuffle(@files);

		foreach my $file (@files)
		{

			my $start = time*1000;

			$0 = 'idleloop - playing: '.$file;
			
			open infile,$ENV{'HOME'}.'/Sites/wallRecords/'.$file or next;


			while(<infile>)
			{
				if(/^(\d+) (.*)\r\n$/)
				{
					my $frame = $2;
					my $delay = $1 - (time*1000-$start);

					if( ($delay > 0) and ($delay < 60000))
					{
						while($delay > 100)
						{
							$delay -= 100;
							usleep(100*1000);
							if($event eq 'next')
							{
								$event = '';
								last;
							}
						}
						usleep($delay*1000);
						
					}
					
					#ceilingLight::binFrame($frame);
					#ceilingLight::readline();
			
					if(length($frame) == 3458)
					{
						pwhd::binFrame($frame);
						pwhd::readline();
					}
				}
				if($event eq 'next')
				{
					$event = '';
					last;
				}
			}
			close infile;
		};
	};
	warn $@ if $@;
}



