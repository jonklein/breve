#!/bin/sh

rm valgrind.txt

for i in `find demos -name '*.tz' -print`; do
	valgrind ./bin/breve_cli -t 100 $i >> valgrind.txt 2>&1
	echo $i
done
