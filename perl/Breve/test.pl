#!/usr/bin/perl -w

use strict;
use Devel::Symdump;

use ExtUtils::testlib;
use Breve;

my @funcs = Devel::Symdump->functions(qw(Breve));
print join(" & \n", @funcs);

#&Breve::testingfoo();
&Breve::brPerlSetController(0,0);

