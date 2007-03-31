SRC	+= perl/breveCallbacksPerl.cc perl/breveFunctionsPerl.cc perl/perlInterface.cc perl/Breve.c

perl/Breve.c: perl/Breve.xs
	@echo Making Perl module.
	xsubpp -C++ perl/Breve.xs > perl/Breve.c
	sed 's/PUTBACK\;//' perl/Breve.c > perl/.Breve.c
	mv perl/.Breve.c perl/Breve.c
