SRC	+= perl/breveCallbacksPerl.cc perl/breveFunctionsPerl.cc perl/perlInterface.cc perl/Breve.c

LDFLAGS += $(shell perl -MExtUtils::Embed -e ldopts)
CPPFLAGS += $(shell perl -MExtUtils::Embed -e ccopts )

perl/Breve.c:
	@echo Making Perl module.
	/usr/bin/perl /usr/share/perl/5.8/ExtUtils/xsubpp  -C++ -typemap /usr/share/perl/5.8/ExtUtils/typemap perl/Breve.xs > perl/Breve.c
	sed --quiet -e 's/PUTBACK\;//' -e 'w perl/.Breve.c' perl/Breve.c
	mv perl/.Breve.c perl/Breve.c
