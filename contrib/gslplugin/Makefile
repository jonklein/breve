#
# Each platform has its own way of building dynamically loaded
# object files.  The target for each platform is derived from
# the output of the command "uname".
#

# Set to location of breve
BREVEPATH=../../bin/breve

all: 
	make -f Makefile `uname`

export

objects: GSLRNG.o

Darwin: 
	make objects 'DEFINES=-DMACOSX'
	$(CC) -bundle -o GSLRNG_plugin.o GSLRNG.o -bundle_loader $(BREVEPATH) -lgsl -lgslcblas

Linux: 
	make objects 'DEFINES=-DLinux'
	$(CC) -shared -o GSLRNG_plugin.o GSLRNG.o

MINGW32_NT-5.1: 
	make objects 'DEFINES=-DWINDOWS'
	dlltool -z GSLRNG.def GSLRNG.o
	$(LD) -shared -o GSLRNG_plugin.o GSLRNG.o GSLRNG.def -lgsl $(BREVEPATH).exe

CYGWIN_NT-5.1: 
	make objects 'DEFINES=-DWINDOWS'
	dlltool -z GSLRNG.def GSLRNG.o
	$(CC) -shared -o GSLRNG_plugin.o GSLRNG.o -L../../lib -lsteve -lutil -lz

.c.o:
	$(CC) -G $(DEFINES) -c $*.c -I.

clean:
	-rm *.o
