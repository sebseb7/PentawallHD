#!/usr/bin/perl

use strict;


my $outbuf;
open infile,$ARGV[0];

while(<infile>)
{
	chomp;

	(my $time, my $str) = split / /;

	if($str =~ /^03(.*)$/)
	{
		my @rgbs;
		my $data = $1;
		foreach my $key (0..575)
		{
			
			$rgbs[$key]  = substr($data,$key*6,6);
		}
		my @newrgbs;

		foreach my $x (0..23)
		{
			foreach my $y (0..23)
			{
				$newrgbs[$y*24+(23-$x)] = $rgbs[$x*24+$y];
			}
		}

		$outbuf.="$time 03".join('',@newrgbs)."\r\n";

	}
	else
	{
		$outbuf.="$time $str\r\n";
	}


}
close infile;

open outfile,'>out.rec';
print outfile $outbuf;
close outfile;
