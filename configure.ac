#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# AC_PREREQ(2.57)

AC_INIT([the breve simulation environment], [2.1], [jk@spiderland.org])
AC_CONFIG_HEADER([include/breve/config.h])

VERSION=2.1

AC_CANONICAL_HOST

AC_PATH_X

case "${host_os}" in
	darwin* )
		dnl Mac OS X configuration

		AUDIOLIBS="-framework CoreAudio -framework AudioToolbox"
		JAVALIBS="-framework JavaVM"
		JAVAINCS="-I/System/Library/Frameworks/JavaVM.framework/Headers"
		GLLIBS="-framework OpenGL -framework GLUT -framework AppKit"
		PLUGINFLAGS="-bundle -undefined suppress -flat_namespace"
		QUICKTIMEFLAGS="-framework QuickTime -framework Carbon"
		AC_DEFINE([MACOSX], [], [Mac OS X])
		ARCHIVER="tar -zcvf"
		ARCHIVE_SUFFIX="tar.gz"
		PLATFORM="macosx"

		;;

	linux* )
		dnl Linux configuration
		GLLIBS="-L$x_libraries -lglut -lGL -lGLU -lXmu -lXt -lSM -lICE -lX11 -lXi -lXext"
		PLUGINFLAGS="-shared"
		AC_DEFINE([LINUX], [], [Linux])
		LDFLAGS="${LDFLAGS} -rdynamic -Xlinker -E"
		ARCHIVER="tar -zcvf"
		ARCHIVE_SUFFIX="tar.gz"
		PLATFORM="linux"

		;;

	mingw* )
		dnl MinGW configuration
		GLLIBS="-lopengl32 -lglut32 -lglu32 -lwsock32 -lstdc++ -liberty"
		JAVALIBS="breve.exe /mingw/lib/libjvm.dll.a jvm.def"
		JAVAINCS="-I/include -I/e/j2sdk1.4.1_03/include -I/e/j2sdk1.4.1_03/include/win32"
		PLUGINFLAGS="-shared"
		AC_DEFINE([MINGW], [], [MinGW])
		AC_DEFINE([WINDOWS], [], [Windows])
		AUDIOLIBS="-lwinmm"
		PTHREADLIBS="-lwsock32"
		ARCHIVER="zip -r"
		ARCHIVE_SUFFIX="zip"
		EXE_SUFFIX=".exe"
		PLATFORM="windows"

		;;

	cygwin* )
		dnl Cygwin configuration
		GLLIBS="-lopengl32 -lglut32 -lglu32 -lwsock32 -lstdc++"
		PLUGINFLAGS="-shared"
		AC_DEFINE([CYGWIN], [], [Cygwin])
		AC_DEFINE([WINDOWS], [], [Windows])
		AUDIOLIBS="-lwinmm"
		ARCHIVER="zip -r"
		ARCHIVE_SUFFIX="zip"
		EXE_SUFFIX=".exe"
		PLATFORM="cygwin"

		;;

	*)
		dnl Unix OSS configuration

		;;
esac

AC_SUBST(AUDIOLIBS)
AC_SUBST(CC)
AC_SUBST(CXX)
AC_SUBST(RANLIB)
AC_SUBST(GLLIBS)
AC_SUBST(JAVALIBS)
AC_SUBST(JAVAINCS)
AC_SUBST(PLUGINFLAGS)
AC_SUBST(QUICKTIMEFLAGS)
AC_SUBST(EXTRALDFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(VERSION)

AC_SUBST(ARCHIVER)
AC_SUBST(ARCHIVE_SUFFIX)
AC_SUBST(EXE_SUFFIX)
AC_SUBST(PLATFORM)

# Checks for programs.
AC_PROG_YACC
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LEX
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB([z], [gzopen],,AC_MSG_FAILURE([Missing zlib library -- see http://www.zlib.org]))
AC_CHECK_LIB([expat], [XML_ParserCreate],,AC_MSG_FAILURE([Missing expat library -- see http://www.jclark.com/xml/expat.html]))
AC_CHECK_LIB([pthread], [pthread_create],,AC_MSG_FAILURE([Missing pthread library]),$PTHREADLIBS)
AC_CHECK_LIB([avcodec], [avcodec_init])
AC_CHECK_LIB([enet], [enet_initialize])
AC_CHECK_LIB([jpeg], [jpeg_read_header])
AC_CHECK_LIB([l], [yywrap])
AC_CHECK_LIB([png], [png_create_write_struct],,,[-lz -lm])
AC_CHECK_LIB([portaudio], [Pa_OpenStream],,,$AUDIOLIBS)
AC_CHECK_LIB([sndfile], [sf_open])
AC_CHECK_LIB([OSMesa], [OSMesaCreateContext],,,$GLLIBS)
AC_CHECK_LIB([termcap], [tputs])
AC_CHECK_LIB([ncurses], [tputs])
AC_CHECK_LIB([history], [add_history])
AC_CHECK_LIB([readline], [readline])

AC_LANG([C++])
AC_CHECK_LIB([ode], [dWorldStep],,AC_MSG_FAILURE([Missing ODE library -- see http://opende.sourceforge.net]),[-lstdc++])
AC_CHECK_LIB([push], [pushEnvironmentNew],,,[-lstdc++])
AC_LANG([C])

# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h float.h limits.h malloc.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/file.h sys/param.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([bzero floor getcwd gethostbyaddr gethostbyname gettimeofday pow regcomp socket sqrt strcasecmp strchr strdup strerror strstr])

AC_CONFIG_FILES([Makefile
                 docBuild/Makefile
                 docBuild/docbook/Makefile
                 graph/Makefile
                 neural/Makefile
                 kernel/Makefile
                 kernel/internalFunctions/Makefile
                 util/Makefile
                 simulation/Makefile
                 java/Makefile
                 steve/Makefile
                 bin/Makefile
                 plugins/slDigitizerPlugin/Makefile
                 plugins/slQTMusicPlugin/Makefile
                 plugins/slRegexPlugin/Makefile
                 plugins/slURLPlugin/Makefile
                 OSX/ConfigureLibraryList
])

if test "$ac_cv_lib_png_png_create_write_struct" != yes; then
	AC_MSG_WARN([Working libpng not found!  Building without PNG support...])
fi

if test "$ac_cv_lib_jpeg_jpeg_read_header" != yes; then
	AC_MSG_WARN([Working libjpeg not found!  Building without JPEG support...])
fi

if test "$ac_cv_lib_portaudio_Pa_OpenStream" != yes; then
	AC_MSG_WARN([Working libportaudio not found!  Building without audio support...])
fi

if test "$ac_cv_lib_sndfile_sf_open" != yes; then
	AC_MSG_WARN([Working libsndfile not found!  Building without audio support...])
fi

if test "$ac_cv_lib_avcodec_avcodec_init" != yes; then
	AC_MSG_WARN([Working libavcodec not found!  Building without movie export support...])
fi

AC_OUTPUT
