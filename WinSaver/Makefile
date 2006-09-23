
WINSAVEROBJS	= WinSaver/WinSaver.o

WinSaver: WinSaverLibs breve.scr

WinSaverLibs:
	rm -rf WinSaver/breve
	mkdir WinSaver/breve
	cp -r lib WinSaver/breve
	rm -f WinSaver/breve/lib/*.a WinSaver/breve/lib/*.lib 
	find WinSaver/breve/lib -name '.svn*' -prune -exec rm -rf {} \;
	cp glut32.dll WinSaver
	cp WinSaver/Creatures.tz WinSaver/breve

breve.scr: lib/libbreve.a $(WINSAVEROBJS)
	$(CXX) -o WinSaver/breve.scr $(WINSAVEROBJS) -Llib -lbreve $(LIBS) -lscrnsave -lgdi32
	
