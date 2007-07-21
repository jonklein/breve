import breve
import sys
import inspect
import xml.dom.minidom
import re
import os.path

sys.path.append( '.' )
sys.path.append( os.path.dirname( sys.argv[ 0 ] ) )

import DocExtract

def WriteDocumentation( xmlstring ):
	xmldocs = xml.dom.minidom.parseString( xmlstring )

	name = xmldocs.documentElement.getAttribute( 'name' )
	parent = xmldocs.documentElement.getAttribute( 'parent' )

	header = ProcessHeader( headertext, name, parent )

	output = file( '%s/%s.html' % ( destination, name ), 'w' )
	output.write( header )
	output.write( '\n' )

	docstring = xmldocs.documentElement.getElementsByTagName( 'classdocstring' )[ 0 ].childNodes[ 0 ].nodeValue

	output.write( '<h3>Class description:</h3><p><blockquote>%s</blockquote>\n' % ProcessDocstring( docstring ) )

	methods = xmldocs.documentElement.getElementsByTagName( 'method' )

	methoddata = []

	for method in methods:
		name = method.getAttribute( 'name' )
		
		if not re.match( '__', name ):
			methoddata.append( ( name, WriteMethod( output, method ) ) )

	output.write( '<h3>Methods:</h3>\n' )

	for method in methoddata:
		output.write( '<li><a href="#%s">%s</a>\n' % ( method[ 0 ], method[ 0 ] ) )

	for method in methoddata:
		output.write( method[ 1 ] )

	output.close()



def WriteMethod( output, xmlnode ):
	name = xmlnode.getAttribute( 'name' )
	isPython = xmlnode.hasAttribute( 'isPython' )

	arguments = xmlnode.getElementsByTagName( 'arguments' )[ 0 ].getElementsByTagName( 'argument' )
	argstring = ''

	argnames = []

	for i in xrange( len( arguments ) ):
		arg  = arguments[ i ].getAttribute( 'name' )

		argstring += arg

		if i != len( arguments ) - 1:
			argstring += ", "

		argnames.append( arg )

	methoddec = '<tt>%s( %s )</tt>' % ( name, argstring )

	docstring = ProcessDocstring( xmlnode.getElementsByTagName( 'docstring' )[ 0 ].childNodes[ 0 ].nodeValue, argnames )

	return '<hr /><a name="%s">%s\n<blockquote><p>%s</p></blockquote>\n' % ( name, methoddec, docstring )

def BuildPythonDeclaration( xmlnode ):
	pass


def ProcessHeader( header, classname, parentname, usage = '' ):
	header = header.replace( '$parent', parentname )
	header = header.replace( '$class', classname )
	header = header.replace( '$usage', usage )

	return header

def ProcessDocstring( docstring, arguments = [] ):
	for i in arguments:
		docstring = re.sub( '(\W)%s(\W)' % i, '\\1<b>%s</b>\\2' % i, docstring )

	docstring = re.sub( 'OBJECT\(([^\)]*)\)', '<a href="\\1.html">\\1</a>', docstring )
	docstring = re.sub( 'METHOD\(([^\)]*)\)', '<a href="#\\1">\\1</a>', docstring )

	return docstring

destination = sys.argv[ 1 ]

#
#
#

header = file( 'docBuild/classheader.html' )
headertext = header.read()


#
#
#

extractor = DocExtract.DocExtract()

for name in breve.__dict__:
	obj = breve.__dict__[ name ]

	if inspect.isclass( obj ) and issubclass( obj, breve.Object ):
		print 'Writing documentation for class "%s"' % name

		xmldoc = extractor.Extract( obj )

		WriteDocumentation( xmldoc.toprettyxml() )



sys.exit( 0 )
