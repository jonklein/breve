AC_PREREQ(2.57)

AC_INIT([the breve simulation environment], [2.1], [jk@spiderland.org])
AC_CONFIG_HEADER([include/breve/config.h])

VERSION=2.1

AC_CANONICAL_HOST

AC_PATH_X

AC_DEFUN([AC_PROG_JAVAC_WORKS],[
AC_CACHE_CHECK([if $JAVAC works], ac_cv_prog_javac_works, [
JAVA_TEST=Test.java
CLASS_TEST=Test.class
cat << \EOF > $JAVA_TEST
/* [#]line __oline__ "configure" */
public class Test {
}
EOF
if AC_TRY_COMMAND($JAVAC $JAVACFLAGS $JAVA_TEST) >/dev/null 2>&1; then
  ac_cv_prog_javac_works=yes
else
  AC_MSG_ERROR([The Java compiler $JAVAC failed (see config.log, check the CLASSPATH?)])
  echo "configure: failed program was:" >&AC_FD_CC
  cat $JAVA_TEST >&AC_FD_CC
fi
rm -f $JAVA_TEST $CLASS_TEST
])
AC_PROVIDE([$0])
])

AC_ARG_VAR(JAVAC, [Java bytecode compiler])
AC_ARG_VAR(JAVAPREFIX, [Path to Java tool-chain])

AC_DEFUN([AC_PROG_JAVAC],[
test "x$JAVAC" = x && AC_CHECK_PROGS(JAVAC, javac "gcj -C" "egcj -C" guavac jikes, :, $PATH:$JAVAPREFIX)
if test "x$JAVAC" = x; then
	AC_MSG_WARN([no acceptable Java compiler found in \$PATH])
	JAVAC=":"
else
	AC_PROG_JAVAC_WORKS
fi
AC_PROVIDE([$0])
])

AC_ARG_VAR(JAR, [Java archiver])

AC_DEFUN([AC_PROG_JAR],[
test "x$JAR" = x && AC_CHECK_PROGS(JAR, jar fastjar ejar, :, $PATH:$JAVAPREFIX)
if test "x$JAR" = x; then
	AC_MSG_WARN([no acceptable jar program found in \$PATH])
	JAR=":"
fi
AC_PROVIDE([$0])
])

case "${host_os}" in
	darwin*)
		AUDIOLIBS="-framework vecLib -framework CoreAudio -framework AudioToolbox"
		JAVALIBS="-framework JavaVM"
		JAVAINCS="-I/System/Library/Frameworks/JavaVM.framework/Headers"
		GLLIBS="-framework OpenGL -framework GLUT -framework AppKit"
		PLUGINFLAGS="-bundle -undefined suppress -flat_namespace"
		QUICKTIMEFLAGS="-framework QuickTime -framework Carbon"
		AC_DEFINE([MACOSX], [], [Mac OS X])
		ARCHIVER="tar czf"
		ARCHIVE_SUFFIX="tar.gz"
		PLATFORM="macosx"

		;;

	linux*)
		GLLIBS="-L$x_libraries -lglut -lGL -lGLU -lXmu -lXt -lSM -lICE -lX11 -lXi -lXext"
		PLUGINFLAGS="-shared"
		AC_DEFINE([LINUX], [], [Linux])
		LDFLAGS="${LDFLAGS} -rdynamic -Xlinker -E"
		ARCHIVER="tar czf"
		ARCHIVE_SUFFIX="tar.gz"
		PLATFORM="linux"

		;;

	mingw*)
		GLLIBS="-lopengl32 -lglut32 -lglu32 -lwsock32 -lstdc++ -liberty"
		JAVALIBS="breve.exe /mingw/lib/libjvm.dll.a jvm.def"
		JAVAINCS="-I/include -I/e/j2sdk1.4.1_03/include -I/e/j2sdk1.4.1_03/include/win32"
		PLUGINFLAGS="-shared"
		PLUGINIMP="-Wl,--out-implib -Wl,lib/plugin.a"
		PLUGINLIB="\$(BREVEDIR)/lib/plugin.a"
		AC_DEFINE([MINGW], [], [MinGW])
		AC_DEFINE([WINDOWS], [], [Windows])
		AUDIOLIBS="-lwinmm"
		PTHREADLIBS="-lwsock32"
		ARCHIVER="zip -r"
		ARCHIVE_SUFFIX="zip"
		EXE_SUFFIX=".exe"
		PLATFORM="windows"

		;;

	cygwin*)
		GLLIBS="-lopengl32 -lglut32 -lglu32 -lwsock32 -lstdc++"
		PLUGINFLAGS="-shared"
		PLUGINIMP="-Wl,--out-implib -Wl,lib/plugin.a"
		PLUGINLIB="\$(BREVEDIR)/lib/plugin.a"
		AC_DEFINE([CYGWIN], [], [Cygwin])
		AC_DEFINE([WINDOWS], [], [Windows])
		AUDIOLIBS="-lwinmm"
		ARCHIVER="zip -r"
		ARCHIVE_SUFFIX="zip"
		EXE_SUFFIX=".exe"
		PLATFORM="cygwin"

		;;
	
	*bsd*)
		GLLIBS="-lglut -lGL -lGLU -lXmu -lXi -lm"
		CPPFLAGS="${CPPFLAGS} -I/usr/local/include -I/usr/X11R6/include"
		LDFLAGS="${LDFLAGS} -Wl,-E -L/usr/local/lib -L/usr/X11R6/lib"
		PLUGINFLAGS="-shared"
		ARCHIVER="tar czf"
		ARCHIVE_SUFFIX="tar.gz"
		PLATFORM=${host_os}

		;;
esac

AC_SUBST(ARCHIVER)
AC_SUBST(ARCHIVE_SUFFIX)
AC_SUBST(AUDIOLIBS)
AC_SUBST(CC)
AC_SUBST(CPPFLAGS)
AC_SUBST(CXX)
AC_SUBST(DLLTOOL)
AC_SUBST(EXE_SUFFIX)
AC_SUBST(GLLIBS)
AC_SUBST(JAVAINCS)
AC_SUBST(JAVALIBS)
AC_SUBST(LDFLAGS)
AC_SUBST(PLATFORM)
AC_SUBST(PLUGINFLAGS)
AC_SUBST(PLUGINIMP)
AC_SUBST(PLUGINLIB)
AC_SUBST(QUICKTIMEFLAGS)
AC_SUBST(RANLIB)
AC_SUBST(REGEXLIB)
AC_SUBST(VERSION)

AC_PROG_CC
AC_PROG_CXX
AC_PROG_JAR
AC_PROG_JAVAC
AC_PROG_LEX
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_YACC

AC_CHECK_TOOL(DLLTOOL, dlltool, :)

AC_PATH_PROG(MKDEP, mkdep)
AC_PATH_PROG(MAKEDEPEND, makedepend)

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
AC_CHECK_LIB([termcap], [tputs])
AC_CHECK_LIB([ncurses], [tputs])
AC_CHECK_LIB([history], [add_history])
AC_CHECK_LIB([readline], [readline])
AC_CHECK_LIB([java], [JNI_CreateJavaVM],,,$JAVALIBS)
AC_CHECK_LIB([gslcblas], [cblas_ccopy])
AC_CHECK_LIB([gslblasnative], [gsl_blas_raw_dgemm])
AC_CHECK_LIB([gsl], [gsl_rng_alloc])
AC_CHECK_LIB([tiff], [TIFFOpen])
AC_CHECK_LIB([regex], [regcomp], [REGEXLIB=-lregex])

case "${host_os}" in
	darwin* )
		LIBS="$LIBS $JAVALIBS"
		AC_DEFINE([HAVE_LIBJAVA])

	;;

	* )
		AC_CHECK_LIB([gslcblas], [cblas_ccopy])
	;;

esac

AC_LANG([C++])
AC_CHECK_LIB([ode], [dWorldStep],,AC_MSG_FAILURE([Missing ODE library -- see http://opende.sourceforge.net]),[-lstdc++])
AC_CHECK_LIB([push], [pushEnvironmentNew],,,[-lstdc++])
AC_CHECK_HEADER([qgame++.h])
AC_CHECK_LIB([qgame++], [main])
AC_LANG([C])

AC_CHECK_LIB([OSMesa], [OSMesaCreateContext],,,$GLLIBS)

AC_FUNC_ALLOCA
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h float.h limits.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/file.h sys/param.h sys/socket.h sys/time.h unistd.h wctype.h])

AC_C_CONST
AC_C_INLINE
AC_HEADER_STDBOOL
AC_HEADER_TIME
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T

AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([floor getcwd gethostbyaddr gethostbyname gettimeofday pow regcomp socket sqrt strcasecmp strchr strdup strerror strstr iswspace])

AC_MSG_CHECKING(how to generate dependency info)
if test "$ac_cv_c_compiler_gnu" = yes; then
	AC_MSG_RESULT($CC -MM)
	MKDEP="$CC -MM"
elif test ! -z "$MKDEP"; then
	AC_MSG_RESULT(mkdep)
	MKDEP="$MKDEP -f -"
elif test ! -z "$MAKEDEPEND"; then
	AC_MSG_RESULT(makedepend)
	MKDEP="$MAKEDEPEND -f-"
else
	AC_MSG_RESULT([nothing suitable.. forget it!])
	MKDEP=":"
fi
AC_SUBST(MKDEP)

AC_CONFIG_FILES([Makefile
		 plugins/slDigitizerPlugin/Makefile
		 plugins/slQTMusicPlugin/Makefile
		 plugins/slRegexPlugin/Makefile
		 plugins/slURLPlugin/Makefile
		 OSX/ConfigureLibraryList
])

if test "$ac_cv_lib_png_png_create_write_struct" != yes; then
	AC_MSG_WARN([libpng not found..  building without PNG support.])
fi

if test "$ac_cv_lib_jpeg_jpeg_read_header" != yes; then
	AC_MSG_WARN([libjpeg not found..  building without JPEG support.])
fi

if test "$ac_cv_lib_portaudio_Pa_OpenStream" != yes; then
	AC_MSG_WARN([libportaudio not found.. building without audio support.])
fi

if test "$ac_cv_lib_sndfile_sf_open" != yes; then
	AC_MSG_WARN([libsndfile not found.. building without audio support.])
fi

if test "$ac_cv_lib_avcodec_avcodec_init" != yes; then
	AC_MSG_WARN([libavcodec not found.. building without movie export support.])
fi

AC_OUTPUT
