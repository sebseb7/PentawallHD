#!/usr/bin/perl -I/Users/k-ot/PentawallHD/wallServer

use pwhd;
use strict;
use Time::HiRes qw(usleep time);
use List::Util qw(shuffle);


pwhd::init();


my $PATH = "/Users/k-ot/Sites/wallRecords";
$0 = 'idleloop';
my $event = '';
my $count;
while(1)
{
	eval
	{

		pwhd::setLevel(0);

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
			
			open infile,'/Users/k-ot/Sites/wallRecords/'.$file or next;


			while(<infile>)
			{
				if(/^(\d+) (.*)\r\n$/)
				{
					my $frame = $2;
					my $delay = $1 - (time*1000-$start);
					if($count<10)
					{
						$count++;
					}
					else
					{
						pwhd::readline();
					}
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
					
					pwhd::binFrame($frame);
				}
				if($event eq 'next')
				{
					$event = '';
					last;
				}
			}
			close infile;
			# black frame
			usleep(500000);
		};
	};
	warn $@ if $@;
}



