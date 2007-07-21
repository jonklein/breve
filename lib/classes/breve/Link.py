
import breve

class Link( breve.Mobile ):
	'''Link objects are physically simulated subclasses of Mobile.  Link objects may be used in conjunction with OBJECT(Joint) objects, to become pieces  of an articulated body.   <P> Links may also be associated with OBJECT(MultiBody) objects, <b>but this is  not required</b> (as they were prior to breve 2.0). <P> In breve 2.0 and later, Link is a subclass of OBJECT(Mobile), and  OBJECT(MultiBody)  objects can be constructed out of any Mobile objects.  This means that a  great deal of the methods and functionality that used to be a part of  OBJECT(Link) are now accessed via the superclasses OBJECT(Mobile) and  OBJECT(Real).'''

	def __init__( self ):
		breve.Mobile.__init__( self )
		Link.init( self )

	def dearchive( self ):

		return breve.Mobile.dearchive( self )

	def destroy( self ):

		breve.Mobile.destroy( self )

	def getMultibody( self ):
		'''Returns the OBJECT(MultiBody) that this link is a part of, if any.'''


		return breve.breveInternalFunctionFinder.linkGetMultibody( self, self.realWorldPointer )

	def init( self ):

		breve.breveInternalFunctionFinder.realSetCollisionProperties( self, self.realWorldPointer, 0.200000, 0.200000 )
		self.enablePhysics()


breve.Link = Link
# Add our newly created classes to the breve namespace

breve.Links = Link



