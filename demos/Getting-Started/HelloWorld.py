
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class HelloWorld( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )

	def iterate( self ):
		print '''Hello, world!'''
		breve.Control.iterate( self )


breve.HelloWorld = HelloWorld


# Create an instance of our controller object to initialize the simulation

HelloWorld()


