#!/bin/tcsh

foreach i ( $* )
	echo $i
	cat $i | ./addmalloc.pl | ./addfree > $i.out
	mv $i $i.orig
	mv $i.out $i
end
