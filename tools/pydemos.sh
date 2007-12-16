#!/bin/tcsh

foreach i ( `find demos -name '*.tz'`) 

	./bin/breve -x -Y $i

end
