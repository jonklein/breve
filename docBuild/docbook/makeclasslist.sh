#!/usr/bin/perl

print "<chapter>\n";

print "<title>The Built-In breve Class APIs</title>\n";

foreach $i ( `ls $ARGV[ 0 ] | grep html | grep -v classlist | grep -v index` ) {
	chop $i;
	$class = `echo $i | cut -d. -f1`;
	chop $class;
	print "<sect1 id=\"${class}\"><title>${class}</title>\n";
	print "<para>\n";
	print "<indexterm><primary>${class} class</primary></indexterm>\n";
	print "Full API documentation for the class <function>$class</function> can be found <ulink url=\"http://www.spiderland.org/documentation/classes/${i}\">here</ulink>.\n";
	print "</para>\n";
	print "</sect1>\n";
}

print "</chapter>";
