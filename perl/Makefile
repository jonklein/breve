SRC	+= perl/breveCallbacksPerl.cc perl/breveFunctionsPerl.cc perl/perlInterface.cc perl/Breve.c

LDFLAGS += $(shell perl -MExtUtils::Embed -e ldopts)
CPPFLAGS += $(shell perl -MExtUtils::Embed -e ccopts )


ALL += perlmodule

perlmodule:
	@echo Making Perl module.
	/usr/bin/perl /usr/share/perl/5.8/ExtUtils/xsubpp  -C++ -typemap /usr/share/perl/5.8/ExtUtils/typemap -typemap perlobject.map Breve.xs > Breve.c
	sed --quiet -e 's/PUTBACK\;//' -e 'w .Breve.c' Breve.c
	mv .Breve.c Breve.c
