#!/usr/bin/perl

if(!open(LICENSE, $ARGV[0])) {
	print "Cannot open $ARGV[0]\n";
	exit(1);
}

$lic = join('', <LICENSE>);
chop $lic;

close(LICENSE);

while(<STDIN>) {
	chop;
	$file = $_;

	if(!open(FILE, $file)) {
		print "skipping $file, cannot open: $!\n";
		next;
	}

	$line = join('', <FILE>);

	close(FILE);

	system("/bin/mv $file $file.no_lic");

	if(!open(FILE, ">$file")) {
		print "cannot write to $file: $!\n";
		next;
	}

	$line =~ s{ /\*.*?Copyright.*?\*/ } []gsx;

	print FILE $lic;
	print FILE "\n\n";
	print FILE $line;

	close(FILE);
}
