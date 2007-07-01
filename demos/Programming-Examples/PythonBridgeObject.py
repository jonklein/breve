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
		# We must call the breve.Object __init__ method before accessing any breve functionality

		breve.Object.__init__( self )
		print "Inited Python bridge object"

	def iterate( self ):
		print "Iterating Python bridge object at time %f" % self.controller.get_time()

	def test_number( self, number ):
		"A simple example of a number passing function callable from steve"
		return number * 2

	def test_vector( self, vector ):
		"A simple example of a vector passing function callable from steve"
		return vector * 2

	def test_list( self, list ):
		"A simple example of a list passing function callable from steve"
		list.reverse()
		return list
