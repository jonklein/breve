TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

LIBS	+= -L../lib -lbreve -lqgame++ -lpush -lode -lavcodec -lgsl -lgslcblas -lreadline -lhistory -lncurses -ltermcap -lsndfile -lportaudio -ltiff -ljpeg -lpng -lexpat -lz -pthread -lm -lglut -lGLU -lGL -lXmu -lXi  -lX11  -lSM -lICE

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

IMAGES	= app_icon.png \
	pause.png \
	play.png \
	stop.png

# release








LEXSOURCES	+= brqtQuickparser.l

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}



