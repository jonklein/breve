#
# PythonBridgeObject.py
# 1/20/2007
# jon klein < jk [at] spiderland.org > 
#
# This is a simple example of a breve compatible Python object. 
#
# In order to be breve compatible, we first import the breve module.
# The breve-compatible object must inherit from the class breve.Object
# (or one of its subclasses).
#

import breve

class PythonBridgeObject( breve.Object ):
	def __init__( self ):
		print "Inited Python bridge object"

		breve.Object.__init__( self )

	def iterate( self ):
		print "Iterating Python bridge object at time %f" % self.controller.get_time()

	def testNumber( self, number ):
		"A simple example of a number passing function callable from steve"
		return number * 2

	def testVector( self, vector ):
		"A simple example of a vector passing function callable from steve"
		return vector * 2

	def testList( self, list ):
		"A simple example of a list passing function callable from steve"
		list.reverse()
		return list
