Notes about the breve simulation library source code, version 2.1

The breve simulation library is in the process of being translated
to C++ in an effort to clean up and formalize certain aspects of 
the library.

The specific goals are:

+ remove gratuitous use of void pointers and typecasts by using 
  templates and STL classes.

+ create a standard interface for adding new elements to simulations.


These two goals are accomplished in large part by replacing fields
in some structures, and by translating some structures to C++ 
classes with virtual methods.  Fulfilling these goals did not 
require translating all structures to C++ classes and using a 
truly C++ oriented design.

As a result, several aspects of the source code are--for the moment--
somewhere between C and C++ design.  These issues will be addressed 
in time.

+ most class computation is still done by C functions which treat
  objects like structs, instead of using C++ methods

+ most classes have all public fields, like C structs.

+ many parent classes have a flag indicating the class type, as was
  required in C.  in many cases, these flags are still used, which 
  is a rather un-C++ design.
