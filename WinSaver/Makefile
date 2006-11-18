
WINSAVEROBJS	= WinSaver/WinSaver.o WinSaver/WinSaverResource.o

WinSaver: WinSaverLibs breve.scr

WinSaverLibs:
	rm -rf WinSaver/breve
	mkdir WinSaver/breve
	cp -r lib WinSaver/breve
	rm -f WinSaver/breve/lib/*.a WinSaver/breve/lib/*.lib 
	find WinSaver/breve/lib -name '.svn*' -prune -exec rm -rf {} \;
	cp glut32.dll WinSaver
	cp WinSaver/Creatures.tz WinSaver/breve

WinSaver/WinSaverResource.o: WinSaver/WinSaverResource.rc
	i686-pc-mingw32-windres -o WinSaver/WinSaverResource.o WinSaver/WinSaverResource.rc

breve.scr: lib/libbreve.a $(WINSAVEROBJS)
	$(CXX) -Wl,--subsystem,windows,-e,_mainCRTStartup -o WinSaver/breve.scr $(WINSAVEROBJS) -Llib -lbreve $(LIBS) -lscrnsave -lgdi32 bin/python23.dll
	
