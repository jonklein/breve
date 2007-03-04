
import breve

class Abstract( breve.Object ):
	'''Abstract is a logical separation of the OBJECT(Object) class, used for objects which have no representation in the simulated  world.  This is in contrast to OBJECT(Real) objects, which  do have representations in the simulated world. Since "abstract" is only a logical distinction, the Abstract class  currently has no methods implemented.  '''

	__slots__ = [ ]

	def __init__( self ):
		breve.Object.__init__( self )

	def dearchive( self ):
		''''''


		return 1



breve.Abstract = Abstract
breve.Abstracts = Abstract



