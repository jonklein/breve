
import breve

class Data( breve.Abstract ):
	'''<b>The use of this class is deprecated and will be removed in a future release.</b>. See the section  <a href="http://www.spiderland.org/breve/docs/docs/archiving.html">Archiving and Dearchiving</a> of  the breve documentation for more details. <p> The now obsolete class description follows. <p> The Data class is a special class that can be saved to disk, loaded in  to simulations and otherwise manipulated in ways that other classes  cannot.  Because of the features of this class, it may only contain  variables of type int, float or vector. <p> Previous versions of breve used a binary format for saving and loading data objects--the current version saves as XML files.  A method for  loading binary objects from previous versions still exists METHOD(load-from-file),  but its use is deprecated.'''

	def __init__( self ):
		breve.Abstract.__init__( self )
		Data.init( self )

	def copyContents( self, otherInstance ):
		'''Copies the contents of this data object to otherInstance.  The instances must be of the same type.'''

		breve.breveInternalFunctionFinder.dataCopyObject( self, otherInstance )

	def init( self ):
		''''''

		if ( breve.breveInternalFunctionFinder.dataCheckVariables( self) != 0 ):
			raise Exception( '''subclasses of class Data may not variables of type object or pointer.''' )



	def loadFrom( self, theFile ):
		'''Deprecated.'''

		return breve.breveInternalFunctionFinder.dataReadObject( self, theFile )


	def loadFromXml( self, theFile ):
		'''Loads the contents of saved XML file theFile into this object.  The XML file must contain a previously archived instance of the same class (archived using METHOD(save-as-xml)).'''

		breve.breveInternalFunctionFinder.dataReadXMLObject( self, theFile )

	def loadWithDialog( self ):
		'''Presents the user with a save dialog box in order to get a  filename, and then loads the contents of the specified filename into this instance.  The chosen file must contain a previously saved instance of the same object. <p> <b>Previous versions of breve used a binary save of the object.  The current version loads files saved in the XML file format.</b>'''

		return breve.breveInternalFunctionFinder.dataReadObjectWithDialog( self)


	def saveAsXml( self, theFile ):
		'''Saves the contents of this instance to an XML file called theFile.  It can later be loaded back into another instance of the same class using  METHOD(load-from-xml).'''

		breve.breveInternalFunctionFinder.dataWriteXMLObject( self, theFile )

	def saveTo( self, theFile ):
		'''Deprecated.'''

		return breve.breveInternalFunctionFinder.dataWriteObject( self, theFile )


	def saveWithDialog( self ):
		'''Presents the user with a save dialog box in order to get a  filename, and then saves the contents of this instance to  the specified filename. <p> <b>Previous versions of breve used a binary save of the object.  The current version saves using an XML file format.</b>'''

		return breve.breveInternalFunctionFinder.dataWriteObjectWithDialog( self)


	def send( self, theHost, thePort ):
		'''Used in conjunction with OBJECT(NetworkServer) to send the contents of this object over the network.'''

		return breve.breveInternalFunctionFinder.sendXMLObject( self, theHost, thePort, self )



breve.Data = Data


