
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


