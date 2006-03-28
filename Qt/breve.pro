TEMPLATE        = app
LANGUAGE        = C++

CONFIG  += qt warn_on release opengl thread

REQUIRES	= opengl

LIBS    += -L../lib -lbreve  -lode -lgsl -lgsl -lpng -lpush -lavformat -lavcodec -lavutil -lreadline -lncurses -ltermcap -lsndfile -lportaudio -ljpeg -lexpat -lz  -lm  -framework OpenGL -framework GLUT -framework AppKit -framework vecLib -framework CoreAudio -framework AudioToolbox  -framework JavaVM

mac:LIBS        += -framework OpenGL

INCLUDEPATH     += ../include/breve

HEADERS += breveGLWidget.h \
        brqtEngine.h \
        brqtMethodPopup.h \
        brqtSteveSyntaxHighlighter.h

SOURCES += breveGLWidget.cpp \
        main.cpp \
        brqtEngine.cpp \
        brqtMethodPopup.cpp

FORMS   = brqtMainWindow.ui \
        brqtLogWindow.ui \
        brqtObjectInspector.ui \
        brqtEditorWindow.ui \
        brqtErrorWindow.ui

IMAGES  = app_icon.png \
        pause.png \
        play.png \
        stop.png


LEXSOURCES      += brqtQuickparser.l

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
