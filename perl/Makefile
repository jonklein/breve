SRC	+= perl/breveCallbacksPerl.cc perl/breveFunctionsPerl.cc perl/perlInterface.cc
LDFLAGS += $(shell perl -MExtUtils::Embed -e ldopts)
CPPFLAGS += $(shell perl -MExtUtils::Embed -e ccopts )

perlmodule:
	@echo Making Perl module.
	cd perl/Breve/; perl Makefile.PL; make; cd ../..;
