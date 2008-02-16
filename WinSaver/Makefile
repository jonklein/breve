
WINSAVEROBJS	= WinSaver/WinSaver.o WinSaver/WinSaverResource.o

WinSaver: WinSaverLibs breve.scr

WinSaverLibs:
	rm -rf WinSaver/breve
	mkdir -p WinSaver/breve/lib
	cp -r lib/classes WinSaver/breve/lib
	touch WinSaver/breve/lib/classes/breveCreatures.xml
	find WinSaver/breve/lib -name '.svn*' -prune -exec rm -rf {} \;
	cp bin/glut32.dll WinSaver
	cp bin/python23.dll WinSaver
	cp WinSaver/Creatures.tz WinSaver/breve

WinSaver/WinSaverResource.o: WinSaver/WinSaverResource.rc
	i686-pc-mingw32-windres -o WinSaver/WinSaverResource.o WinSaver/WinSaverResource.rc

breve.scr: lib/libbreve.a $(WINSAVEROBJS)
	$(CXX) -Wl,--subsystem,windows,-e,_mainCRTStartup -o WinSaver/breve.scr $(WINSAVEROBJS) -Llib -lbreve $(LIBS) -lscrnsave -lgdi32 bin/python23.dll
	
