#
# This plugin was written by Raphael Crawford-Marks and David Prude.
# To build this plugin, you must have an SNNS distribution built in 
# this directory.
#

all: breveSNNS.so.1.o

breveSNNS.so.1.o: breveSNNS.o
	# $(LD) -shared -o breveSNNS.so.1.o ./breveSNNS.o ./kernel/*.o
	$(CC) -bundle -undefined suppress -flat_namespace -o breveSNNS.so.1.o ./breveSNNS.o $(SNNSDIR)/*.o 

.c.o:
	$(CC) -g -c $*.c -I../../plugins

clean:
	-rm *.o
