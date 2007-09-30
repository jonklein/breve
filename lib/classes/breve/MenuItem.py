
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class MenuItem( breve.Abstract ):
	'''The MenuItem class holds menu items associated with objects.  Menus can be associated with Mobile objects, in which case they are shown as contextual menu items, or associated with Control objects in which case they are shown under the global application menu.'''

	def __init__( self ):
		breve.Abstract.__init__( self )
		self.checked = 0
		self.enabled = 0
		self.menuPointer = None
		self.method = ''
		self.name = ''
		self.owner = None

	def check( self ):
		'''Places a check mark next to the menu item.'''

		if ( not self.menuPointer ):
			return

		breve.breveInternalFunctionFinder.menuItemSetCheck( self, self.menuPointer, 1 )
		self.checked = 1

	def createMenu( self, menuName, theObject, methodName ):
		'''This method initializes a menu item with title menuName for theObject which will call methodName when selected.'''

		self.name = menuName
		self.method = methodName
		self.owner = theObject
		self.owner.addDependency( self )
		self.menuPointer = breve.breveInternalFunctionFinder.menuItemNew( self, self.owner, self.method, self.name )
		if ( not self.menuPointer ):
			print '''error adding menu item for method %s.''' % (  methodName )
			breve.deleteInstances( self )
			self.enabled = 1
			return 0


		return self

	def dearchive( self ):
		self.menuPointer = breve.breveInternalFunctionFinder.menuItemNew( self, self.owner, self.method, self.name )
		if self.checked:
			self.check()
		else:
			self.uncheck()

		if self.enabled:
			self.enable()
		else:
			self.disable()

		return 1

	def disable( self ):
		'''Disables the menu item such that it cannot be selected.'''

		if ( not self.menuPointer ):
			return

		breve.breveInternalFunctionFinder.menuItemSetEnabled( self, self.menuPointer, 0 )
		self.enabled = 0

	def enable( self ):
		'''Enables a menu item, if it is disabled.'''

		if ( not self.menuPointer ):
			return

		breve.breveInternalFunctionFinder.menuItemSetEnabled( self, self.menuPointer, 1 )
		self.enabled = 1

	def getDescription( self ):
		return self.name

	def uncheck( self ):
		'''Removes the check mark next to the menu item, if it exists.'''

		if ( not self.menuPointer ):
			return

		breve.breveInternalFunctionFinder.menuItemSetCheck( self, self.menuPointer, 0 )
		self.checked = 0


breve.MenuItem = MenuItem
# Add our newly created classes to the breve namespace

breve.MenuItems = MenuItem



