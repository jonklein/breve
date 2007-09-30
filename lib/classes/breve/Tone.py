
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class Tone( breve.Abstract ):
	'''A Tone is an object which plays a constant tone.  You can set  the volume, frequency and left-right balance of the tone.  Creating the object will automatically start playing the tone.  Deleting the  object will cause the tone to stop playing. <p> A tone can also be effectively stopped by setting its volume to 0.'''

	def __init__( self ):
		breve.Abstract.__init__( self )
		self.tonePointer = None
		Tone.init( self )

	def destroy( self ):
		if self.tonePointer:
			breve.breveInternalFunctionFinder.freeSinewave( self, self.tonePointer )


	def init( self ):
		self.tonePointer = breve.breveInternalFunctionFinder.newSinewave( self, 0 )

	def setBalance( self, newBalance ):
		'''Sets the left/right balance of the sinewave to newBalance, a  value between 0 and 1.  .5 is perfectly balanced, while 1.0 plays the tone completely on the right channel and 0.0 plays the tone entirely on the left channel.'''

		if self.tonePointer:
			breve.breveInternalFunctionFinder.setBalance( self, self.tonePointer, newBalance )


	def setFrequency( self, newFrequency ):
		'''Sets the frequency of the tone to newFrequency.'''

		if self.tonePointer:
			breve.breveInternalFunctionFinder.setFrequency( self, self.tonePointer, newFrequency )


	def setVolume( self, newVolume ):
		'''Sets the volume of the tone to newVolume.  Volume is specified on a scale from 0.0 to 1.0.'''

		if ( newVolume > 1.000000 ):
			newVolume = 1.000000
		else:
			if ( newVolume < 0.000000 ):
				newVolume = 0.000000


		if self.tonePointer:
			breve.breveInternalFunctionFinder.setVolume( self, self.tonePointer, newVolume )



breve.Tone = Tone
# Add our newly created classes to the breve namespace

breve.Tones = Tone



