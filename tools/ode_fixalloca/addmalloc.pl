#!/usr/bin/perl

print "#define NO_STACK_ALLOC\n";

while(<STDIN>) {
	$line = $_;

	s/\/\/.*//;

	if(/(\w+)\s*=.*ALLOCA/i) {
		if($lastmark == 0) {
			print "#ifndef NO_STACK_ALLOC\n"
		}

		print "// ALLOC_MARK:$1\n";
		print $line;

		push @lines, $line;

		$lastmark = 1;
	} else {
		if($lastmark == 1) {
			print "#else\n";
		
			foreach $v (@lines) {
				$v =~ s/ALLOCA/malloc/i;
				print $v;
			}

			print "#endif /* NO_STACK_ALLOC */\n";

			@lines = ( );
		}

		print $line;
		$lastmark = 0;
	}
}
