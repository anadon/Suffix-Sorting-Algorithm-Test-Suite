#! /usr/bin/perl

use strict;
use Time::HiRes;
require Time::HiRes;
use Benchmark ":hireswallclock";
$Benchmark::Default_Format='5.3f';

my $format='.3f';
my $loop = 5;
my $command = "./suftest";
my $file = shift;

my $r = timeit($loop, sub{`"$command" "$file"`});
if(!$?) { printf("%$format\n", ($$r[3] + $$r[4]) / $$r[5]); }
else    { printf("Suftest error\n"); }

1;
__END__
