#!/bin/tcsh

echo "<itemizedlist>"

foreach i (`ls $1 | grep html | grep -v classlist | grep -v index`)
	set class = `echo $i | cut -d. -f1`
	echo "<listitem><para>"
	echo "<indexterm><primary>${class} class</primary></indexterm>"
	echo "<ulink url="\"../classes/${i}\"">${class}</ulink></para>"
	echo "</listitem>"
end

echo "</itemizedlist>"
