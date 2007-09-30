
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class Abstract( breve.Object ):
	'''Abstract is a logical separation of the OBJECT(Object) class, used for objects which have no representation in the simulated  world.  This is in contrast to OBJECT(Real) objects, which  do have representations in the simulated world. Since "abstract" is only a logical distinction, the Abstract class  currently has no methods implemented.  '''

	def __init__( self ):
		breve.Object.__init__( self )

	def dearchive( self ):
		return 1


breve.Abstract = Abstract
# Add our newly created classes to the breve namespace

breve.Abstracts = Abstract



