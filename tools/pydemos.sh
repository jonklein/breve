#!/bin/tcsh

foreach i ( `find demos -name '*.tz'`) 

	./bin/breve -Y $i

end
