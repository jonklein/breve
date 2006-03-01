TEMPLATE        = app
LANGUAGE        = C++

// CONFIG  += qt warn_on release opengl thread

QT		+= opengl release

LIBS    += -L../lib -lbreve -framework JavaVM -lode -lgsl -lpng -lftgl -lqgame++ -lpush -lavformat -lavcodec -lavutil -lftgl -l3ds -lreadline -lhistory -lncurses -ltermcap -lsndfile -lportaudio -ltiff -ljpeg -lenet -lexpat -lz  -lm -framework OpenGL -framework GLUT -framework AppKit -framework vecLib -framework CoreAudio -framework AudioToolbox 

# mac:LIBS	+= -framework OpenGL

INCLUDEPATH	+= ../include/breve

HEADERS += \
		brqtGLWidget.h \
        brqtEngine.h \
        brqtMainWindow.h \
        brqtMethodPopup.h \
        brqtEditorWindow.h \
        brqtSteveSyntaxHighlighter.h \
        brqtWidgetPalette.h \
        brqtMoveableWidgets.h

FORMS   = \
		brqtMainWindow.ui \
#        brqtLogWindow.ui \
#        brqtObjectInspector.ui \
		brqtEditorWindow.ui \
		brqtWidgetPalette.ui \
		brqtErrorWindow.ui

SOURCES += \
		brqtGLWidget.cpp \
        main.cpp \
        brqtEngine.cpp \
        brqtMainWindow.cpp \
        brqtEditorWindow.cpp \
        brqtMoveableWidgets.cpp \
		brqtWidgetPalette.cpp \
        brqtMethodPopup.cpp

LEXSOURCES      += brqtQuickparser.l

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
