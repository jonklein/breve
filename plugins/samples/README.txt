   Sample Plugins for The breve Simulation Environment
             http://www.spiderland.org/breve

This sample directory contains two very simple examples of 
building breve plugins.  These examples show trivial examples 
of how C and C++ code can be accessed from breve simulations.

NOTE: Effective as of breve version 2.4, all plugins must be 
compiled with a C++ compiler because of changes to the plugin
API.  **Code can still be written in C, but must be compiled 
with a C++ compiler**.  Additionally, the entry point function
may need to be defined with C linkage as shown in the example
program.

To build the samples, type "make".

Full instructions on creating plugins for breve can be found
in the breve documentation:
	http://www.spiderland.org/breve/docs
