TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on debug
# release

LIBS	+= -L../lib -lbreve -lqgame++ -lpush -lode -framework JavaVM -lavcodec -lgsl -lreadline -lhistory -lncurses -ltermcap -lsndfile -lportaudio -ltiff -ljpeg -lenet -lexpat -lz -lm -framework OpenGL -framework GLUT -framework AppKit -framework vecLib -framework CoreAudio -framework AudioToolbox

mac:LIBS	+= -framework OpenGL

INCLUDEPATH	+= ../include/breve

HEADERS	+= breveGLWidget.h \
	brqtEngine.h \
	brqtMethodPopup.h \
	brqtSteveSyntaxHighlighter.h

SOURCES	+= breveGLWidget.cpp \
	main.cpp \
	brqtEngine.cpp \
	brqtMethodPopup.cpp

FORMS	= brqtMainWindow.ui \
	brqtLogWindow.ui \
	brqtObjectInspector.ui \
	brqtEditorWindow.ui \
	brqtErrorWindow.ui

IMAGES	= images/fileopen \
	images/filesave \
	images/undo \
	images/redo \
	images/editcut \
	images/editcopy \
	images/editpaste \
	images/searchfind \
	app_icon.png \
	pause.png \
	play.png \
	stop.png

LEXSOURCES	+= brqtQuickparser.l

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}



