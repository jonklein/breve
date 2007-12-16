#!/bin/tcsh

foreach i ( `ls lib/classes/*.tz`) 

	./bin/breve -x -Y $i

end

#
# Control & Object have some custom code which does not convert automatically
# 

rm lib/classes/Control.py
rm lib/classes/Object.py

mv lib/classes/*.py lib/classes/breve
