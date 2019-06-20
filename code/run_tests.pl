#!/usr/bin/perl
use strict;
use warnings;

for (my $i=1; $i <= 10; $i++) {
   system("./BinPackTest -i datasets\\c4p3.txt -o logs\\c4p3t$i.txt -p 130 -m 6");
}