#!/bin/sh

if [ "$2" == "" ]; then
	echo "Usage: $0 test-directory output"
	exit 1
fi

VALGRIND=`which valgrind`

rm -f $2

for i in `find $1 -name '*.tz' -print | xargs grep -l '^Controller'`; do
	echo "Running $i..."
	echo "Running $i..." >> $2
	$VALGRIND ./bin/breve_cli -t 10 $i >> $2 2>&1 
done
