SRC	+= perl/breveCallbacksPerl.cc perl/breveFunctionsPerl.cc perl/perlInterface.cc
LDFLAGS += $(shell perl -MExtUtils::Embed -e ldopts)
CPPFLAGS += $(shell perl -MExtUtils::Embed -e ccopts )

ALL += perlmodule

perlmodule:
	@echo Making Perl module.
	cd perl; cd Breve; perl Makefile.PL; make; cd ..; cd ..;
