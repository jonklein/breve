#!/bin/perl
#
# adds windows newlines to a unix text file

foreach $file (@ARGV) {
	print $file;

	if(!open(F, $file)) {
		warn "Cannot open $file: $!\n";
		next;
	}

	if(!open(FOUT, ">$file.out")) {
		warn "Cannot open $file.out: $!\n";
		next;
	}

	while(<F>) {
		s/\n/\r\n/;
		print FOUT;
	}

	close F;
	close FOUT;
}
