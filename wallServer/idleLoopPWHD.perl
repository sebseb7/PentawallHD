#!/usr/bin/perl

use g3d2;
use strict;
use Time::HiRes qw(usleep time);
use List::Util qw(shuffle);


g3d2::init();

close STDOUT;
open STDOUT,'>>logfile.txt';
open STDERR,'>&STDOUT';


my $PATH = "/opt/wallRecords_g3d2";
$0 = 'idleloop';
my $event = '';
while(1)
{
	eval
	{

		g3d2::setLevel(0);

		my @files;
		
		opendir(my $dh, $PATH) || die "can't opendir: $!";
		while(my $file  = readdir($dh)) {
			next if $file !~  /\.rec$/;
			push @files,$file;
		}
		closedir $dh;
#		@files = shuffle(@files);

		foreach my $file (@files)
		{

			my $start = time*1000;

			$0 = 'idleloop - playing: '.$file;
			
			open infile,'/opt/wallRecords_g3d2/'.$file or next;


			while(<infile>)
			{
				if(/^(\d+) (.*)\r\n$/)
				{
					my $delay = $1 - (time*1000-$start);
					if( ($delay > 0) and ($delay < 60000))
					{
						while($delay > 100)
						{
							#warn 'x';
							$delay -= 100;
#							usleep(100*1000);
							if($event eq 'next')
							{
								$event = '';
								last;
							}
						}
#						usleep($delay*1000);
						
					}
					#warn 'a';
					g3d2::binFrame($2);
					#warn 'ok';
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



