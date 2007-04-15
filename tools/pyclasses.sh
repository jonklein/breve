#!/bin/tcsh

foreach i ( `ls lib/classes/*.tz`) 

	./bin/breve -Y $i

end

rm lib/classes/Control.py
rm lib/classes/Object.py

mv lib/classes/*.py lib/classes/breve
