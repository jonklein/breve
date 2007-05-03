
import breve

class Sound( breve.Abstract ):
	'''The Sound class allows sound files to be loaded and played in  a simulation. The sound file, in WAV or AIFF format, is loaded using the  method METHOLD(load).  The sound can then be played using  METHOD(play).  The same sound effect can be played several times simultaneously by making repeated calls to METHOD(play).  '''

	def __init__( self ):
		breve.Abstract.__init__( self )
		self.soundPointer = None

	def destroy( self ):
		''''''


		breve.breveInternalFunctionFinder.freeSoundData( self, self.soundPointer )

	def load( self, file ):
		'''Loads the sound-file filename using the current breve search path.  Returns self.'''


		if self.soundPointer:
			breve.breveInternalFunctionFinder.freeSoundData( self, self.soundPointer )

		self.soundPointer = breve.breveInternalFunctionFinder.loadSoundData( self, file )
		return self

	def play( self, speed = 1.000000 ):
		'''Begins playing this sound.  The optional argument speed specifies  the speed (and accompanying frequency change) of the sound. '''


		if self.soundPointer:
			breve.breveInternalFunctionFinder.playSoundData( self, self.soundPointer, speed )



breve.Sound = Sound
breve.Sounds = Sound



