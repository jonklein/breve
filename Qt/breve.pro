TEMPLATE        = app
LANGUAGE        = C++

CONFIG  += qt warn_on release opengl thread

REQUIRES	= opengl

LIBS    += -L../lib -lbreve  -lode -lgsl -lgsl -lpng -lpush -lavformat -lavcodec -lavutil -lgslcblas -lsndfile -lportaudio -ljpeg -lexpat -lz -lpthread -lm  -lglut32 -lglu32 -lopengl32 -lws2_32 -lwinmm 

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
