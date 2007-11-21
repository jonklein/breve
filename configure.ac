AC_PREREQ(2.57)

AC_INIT([the breve simulation environment], [2.6.1], [jk@spiderland.org])
AC_CONFIG_HEADER([include/breve/config.h])

VERSION=$PACKAGE_VERSION

AC_CANONICAL_HOST

AC_ARG_ENABLE([engineonly], [], engine_only=yes)
AC_ARG_ENABLE([static], [], static=yes)
AC_ARG_ENABLE([perl], [], perl=yes)
AC_ARG_ENABLE([python], [], python=yes)

python=yes

if test "x$static" != "x"; then 
	BEGIN_STATIC="-Xlinker -Bstatic"
	BEGIN_DYNAMIC="-Xlinker -Bdynamic"
fi

AC_CHECK_TOOL(AR, ar)

AC_PROG_CC
AC_PROG_CXX
AC_PROG_LEX
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_YACC

AC_ARG_WITH([wx], AC_HELP_STRING([--with-wx=path], [use <path> for wxWindows headers and libraries]), WXCONFIG=[$withval/bin/wx-config], WXCONFIG=[wx-config])

AC_PATH_PROG(MKDEP, mkdep)
AC_PATH_PROG(MAKEDEPEND, makedepend)

AC_MSG_CHECKING(how to generate dependency info)
if test "$ac_cv_c_compiler_gnu" = yes; then
	AC_MSG_RESULT($CXX -MM)
	MKDEP="$CXX -MM"
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

AC_PATH_XTRA

AC_LANG(C++)

case ${host_os} in
	darwin* )
		custom_cblas="-framework vecLib"

		LIBS="-framework OpenGL -framework GLUT -framework AppKit -framework vecLib -framework CoreAudio -framework AudioToolbox ${LIBS}"
		PLUGINFLAGS="-bundle -undefined suppress -flat_namespace"
		QUICKTIMEFLAGS="-framework QuickTime -framework Carbon"
		ARCHIVER="tar czf"
		ARCHIVE_SUFFIX="tar.gz"
		PLATFORM="macosx"
		AC_DEFINE([MACOSX], [1], [Mac OS X])
		EXTRAOPTFLAGS="-mdynamic-no-pic -falign-loops=16 -ftree-vectorize -funroll-loops"
		;;

	cygwin* | mingw* )
		LIBS="-lglut32 -lglu32 -lopengl32 -lws2_32 -lwinmm `pwd`/bin/python23.dll ${LIBS}"
		PLUGINFLAGS="-shared"
		PLUGINIMP="-Wl,--out-implib -Wl,lib/brevePlugin.lib"
		PLUGINLIB="\$(BREVEDIR)/lib/breveIDEPlugin.lib"
		ARCHIVER="zip -r"
		ARCHIVE_SUFFIX="zip"
		EXE_SUFFIX=".exe"
		PLATFORM="windows"
		AC_DEFINE([WINDOWS], [1], [Windows])
		AC_CHECK_LIB([pthread], [main])

		;;

	linux* | *bsd* )
		LIBS="${LIBS} ${X_LIBS}"
		AC_CHECK_LIB([Xmu], [main],,AC_MSG_FAILURE([Missing Xmu Library]))
		AC_CHECK_LIB([Xi], [main],,AC_MSG_FAILURE([Missing Xi Library]))
		LIBS="${LIBS} ${X_EXTRA_LIBS}"
		AC_CHECK_LIB([X11], [main],,AC_MSG_FAILURE([Missing X11 Library]))

		AC_CHECK_LIB([GL], [glGetIntegerv],,AC_MSG_FAILURE([Missing OpenGL Library]))
		AC_CHECK_LIB([GLU], [gluGetString],,AC_MSG_FAILURE([Missing GLU Library]))
		AC_CHECK_LIB([glut], [glutInit],,AC_MSG_FAILURE([Missing GLUT Library]))

		PLUGINFLAGS="-shared"
		PLUGINIMP="-Wl,-E"
		ARCHIVER="tar czf"
		ARCHIVE_SUFFIX="tar.gz"
		PLATFORM=${host_os}
		;;
esac

#
# Python wants to be dynamically linked as well
#

if test "x$python" != "x"; then 
	AC_CHECK_LIB([python], [Py_Initialize])
	AC_CHECK_LIB([python2.3], [Py_Initialize])
	AC_CHECK_LIB([python23], [Py_Initialize])
	AC_CHECK_LIB([python2.4], [Py_Initialize])
	AC_CHECK_LIB([python24], [Py_Initialize])
	AC_CHECK_LIB([python2.5], [Py_Initialize])
	AC_CHECK_LIB([python25], [Py_Initialize])

	if test "$ac_cv_lib_python2_3_Py_Initialize" = yes; then
		AC_DEFINE( [HAVE_LIBPYTHON], [], [Have the required Python libraries] )
		CPPFLAGS="${CPPFLAGS} -I/usr/include/python2.3 -I/usr/include/python"
	fi

	if test "$ac_cv_lib_python23_Py_Initialize" = yes; then
		AC_DEFINE( [HAVE_LIBPYTHON], [], [Have the required Python libraries] )
		CPPFLAGS="${CPPFLAGS} -I/usr/include/python2.3 -I/usr/include/python"
	fi

	if test "$ac_cv_lib_python2_4_Py_Initialize" = yes; then
		AC_DEFINE( [HAVE_LIBPYTHON], [], [Have the required Python libraries] )
		CPPFLAGS="${CPPFLAGS} -I/usr/include/python2.4 -I/usr/include/python"
	fi

	if test "$ac_cv_lib_python24_Py_Initialize" = yes; then
		AC_DEFINE( [HAVE_LIBPYTHON], [], [Have the required Python libraries] )
		CPPFLAGS="${CPPFLAGS} -I/usr/include/python2.4 -I/usr/include/python"
	fi

	if test "$ac_cv_lib_python2_5_Py_Initialize" = yes; then
		AC_DEFINE( [HAVE_LIBPYTHON], [], [Have the required Python libraries] )
		CPPFLAGS="${CPPFLAGS} -I/usr/include/python2.5 -I/usr/include/python"
	fi

	if test "$ac_cv_lib_python25_Py_Initialize" = yes; then
		AC_DEFINE( [HAVE_LIBPYTHON], [], [Have the required Python libraries] )
		CPPFLAGS="${CPPFLAGS} -I/usr/include/python2.5 -I/usr/include/python"
	fi
fi

#
# Dyanmically linking in Perl
#

HAVEPERL=0

if test "x$perl" != "x"; then 
	OLDCPP=$CPPFLAGS
	OLDLIB=$LIBS

	HAVEPERL=1

	PERLCPP=`perl -MExtUtils::Embed -e ccopts`
	PERLLIB=`perl -MExtUtils::Embed -e ldopts`

	LIBS="$OLDLIB $PERLLIB"
	CPPFLAGS="$OLDCPP $PERLCPP"

	AC_CHECK_LIB([perl], [perl_construct])
	AC_CHECK_PROGS(XSUBPP, [xsubpp], [], $PATH )

	#
	# Use Perl if we have the perl libraries, locate xsubpp and if we're not cross-compiling
	#

	if test "$ac_cv_lib_perl_perl_construct" = yes && test "x$ac_cv_prog_XSUBPP" != "x" && test "$build" = "$host"; then
		# if we do have Perl, we want to clobber the latest addition to $LIBS, because the proper
		# perl flags will appear in $PERLLIB
		LIBS="$OLDLIB $PERLLIB"
		CPPFLAGS="$OLDCPP $PERLCPP"
	else
		# perl linking failed -- restore the non-perl libraries 
		LIBS=$OLDLIB
		CPPFLAGS=$OLDCPP
	fi
fi


if test "x$engine_only" = "x"; then
	if test "$ECL_CONFIG" != ""; then
		LDECL=`$ECL_CONFIG --libs`
		LDFLAGS="$LDFLAGS $LDECL"
		CPPECL=`$ECL_CONFIG --cflags`        
		AC_CHECK_LIB([eclgc], [main])
		AC_CHECK_LIB([sockets], [main])
		AC_CHECK_LIB([cmp], [main])
		AC_CHECK_LIB([eclgmp], [main])
		AC_CHECK_LIB([ecl], [cl_boot])
	fi    
fi

#
# These required system libraries are assumed to require dynamic linking even 
# when static linking is requested.  Other (optional) libraries may be linked
# statically if present and if requested with --enable-static
#

LIBS="$BEGIN_DYNAMIC $LIBS"

case ${host_os} in
	*bsd* )
		CPPFLAGS="${CPPFLAGS} -I/usr/local/include ${X_CFLAGS}"
		LDFLAGS="${LDFLAGS} -L/usr/local/lib ${X_LIBS}"
		;;

	linux* )
		CPPFLAGS="${CPPFLAGS} ${X_CFLAGS}"
		LDFLAGS="${LDFLAGS} ${X_LIBS}"
		;;

	mingw* )
		AC_DEFINE([MINGW], [1], [MinGW])
		;;
esac

AC_SUBST(ARCHIVER)
AC_SUBST(ARCHIVE_SUFFIX)
AC_SUBST(CC)
AC_SUBST(AR)
AC_SUBST(CPPFLAGS)
AC_SUBST(CXX)
AC_SUBST(EXE_SUFFIX)
AC_SUBST(LDFLAGS)
AC_SUBST(PLATFORM)
AC_SUBST(HAVEPERL)
AC_SUBST(PLUGINFLAGS)
AC_SUBST(PLUGINIMP)
AC_SUBST(PLUGINLIB)
AC_SUBST(QUICKTIMEFLAGS)
AC_SUBST(RANLIB)
AC_SUBST(REGEXLIB)
AC_SUBST(VERSION)
AC_SUBST(EXTRAOPTFLAGS)
AC_SUBST(WXCONFIG)

AC_HEADER_TIME
AC_CHECK_HEADERS([sys/time.h wctype.h])

AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

AC_CHECK_LIB([m], [isnan])

AC_CHECK_HEADERS(pthread.h)

ol_link_threads=no

if test $ac_cv_header_pthread_h = yes ; then
	OL_POSIX_THREAD_VERSION

	if test $ol_cv_pthread_version != 0 ; then
		AC_DEFINE_UNQUOTED(HAVE_PTHREADS,$ol_cv_pthread_version,
			[define to pthreads API spec revision])
	else
		AC_MSG_ERROR([unknown pthread version])
	fi

	ol_with_threads=found

	OL_HEADER_LINUX_THREADS
	OL_HEADER_GNU_PTH_PTHREAD_H

	if test $ol_cv_header_gnu_pth_pthread_h = no ; then
		AC_CHECK_HEADERS(sched.h)
	fi

	AC_CACHE_CHECK([for pthread_create in default libraries],
		ol_cv_pthread_create,[
	AC_TRY_RUN(OL_PTHREAD_TEST_PROGRAM,
		[ol_cv_pthread_create=yes],
		[ol_cv_pthread_create=no],
		[AC_TRY_LINK(OL_PTHREAD_TEST_INCLUDES,OL_PTHREAD_TEST_FUNCTION,
			[ol_cv_pthread_create=yes],
			[ol_cv_pthread_create=no])])])

	if test $ol_cv_pthread_create != no ; then
		ol_link_threads=posix
		ol_link_pthreads=""
	fi

	OL_PTHREAD_TRY([-kthread],	[ol_cv_pthread_kthread])
	OL_PTHREAD_TRY([-pthread],	[ol_cv_pthread_pthread])
	OL_PTHREAD_TRY([-lpthread],	[ol_cv_pthread_lpthread])
	OL_PTHREAD_TRY([-lc_r],		[ol_cv_pthread_lc_r])
	OL_PTHREAD_TRY([-lpthreads],	[ol_cv_pthread_lib_lpthreads])

	LIBS="$ol_link_pthreads $LIBS"
fi

AC_CHECK_LIB([z], [gzopen],,AC_MSG_FAILURE([Missing zlib library -- see http://www.zlib.org]))
AC_CHECK_LIB([regex], [regcomp], [REGEXLIB=-lregex])
AC_CHECK_LIB([expat], [XML_ParserCreate],,AC_MSG_FAILURE([Missing expat library -- see http://www.jclark.com/xml/expat.html]))

# the engine_only flag means to ignore all of the extra libraries.

if test "x$engine_only" = "x"; then
	AC_CHECK_LIB([enet], [enet_initialize])
	AC_CHECK_LIB([jpeg], [jpeg_read_header])
	AC_CHECK_LIB([tiff], [TIFFOpen])

	AC_CHECK_LIB([sndfile], [sf_open])

	if test "$ac_cv_lib_sndfile_sf_open" = yes; then 
		# only consider portaudio if we have libsndfile
		AC_CHECK_LIB([portaudio], [Pa_OpenStream])
		AC_CHECK_LIB([FLAC], [FLAC__seekable_stream_encoder_init])
	fi

	AC_CHECK_LIB([termcap], [tputs])
	// AC_CHECK_LIB([ncurses], [tputs])
	AC_CHECK_LIB([history], [add_history])
	AC_CHECK_LIB([readline], [readline])
	AC_CHECK_LIB([3ds], [lib3ds_io_new])

	AC_CHECK_LIB([freetype], [FT_Init_FreeType])

	if test "$ac_cv_lib_freetype_FT_Init_FreeType" = yes; then 
		# only consider ftgl if we have freetype
		AC_CHECK_LIB([ftgl], [main])
	fi

	AC_CHECK_LIB([avutil], [main])
	AC_CHECK_LIB([avcodec], [avcodec_init])
	AC_CHECK_LIB([avformat], [av_write_frame])

	AC_CHECK_LIB([push], [pushEnvironmentNew])
	AC_CHECK_HEADER([qgame++.h])

	# look for main in the C++ only libraries, since the function names are mangled.

	AC_CHECK_LIB([qgame++], [main])

	AC_CHECK_PROGS(ECL_CONFIG, [ecl-config], [], $PATH:/usr/local/bin:/usr/bin:/usr/local/ecl )
fi

if test "x$custom_cblas" = x; then
	AC_CHECK_LIB([gslcblas], [cblas_ccopy])
fi

AC_CHECK_LIB([png], [png_create_write_struct])
AC_CHECK_LIB([gsl], [gsl_rng_alloc],,AC_MSG_FAILURE([Missing GSL library -- see http://www.gnu.org/software/gsl/]))
AC_CHECK_LIB([ode], [dWorldStep],,AC_MSG_FAILURE([Missing ODE library -- see http://opende.sourceforge.net]))
AC_CHECK_LIB([curl], [curl_easy_init])

AC_CHECK_FUNCS([getopt_long gettimeofday iswspace mkstemp strdup vasprintf])

_old_cppflags=${CPPFLAGS}
_old_ldflags=${LDFLAGS}
_old_libs=${LIBS}

#
# If static linking is requested, add a flag for all non-system libraries.  The
# system libraries have alreay been set to allow dynamic linking.
# 

LIBS="$BEGIN_STATIC $LIBS"

AC_CONFIG_FILES([Makefile
		 plugins/slDigitizerPlugin/Makefile
		 plugins/slQTMusicPlugin/Makefile
		 plugins/slRegexPlugin/Makefile
		 plugins/samples/Makefile
		 OSX/ConfigureLibraryList
		 Qt/breve.pro
		 wx/Makefile
])

if test "$ac_cv_lib_png_png_create_write_struct" != yes; then
	AC_MSG_WARN([libpng not found. building without PNG support.])
fi

if test "$ac_cv_lib_jpeg_jpeg_read_header" != yes; then
	AC_MSG_WARN([libjpeg not found. building without JPEG support.])
fi

if test "$ac_cv_lib_portaudio_Pa_OpenStream" != yes; then
	AC_MSG_WARN([libportaudio not found. building without audio support.])
fi

if test "$ac_cv_lib_sndfile_sf_open" != yes; then
	AC_MSG_WARN([libsndfile not found. building without audio support.])
fi

if test "$ac_cv_lib_avformat_av_write_frame" != yes; then
	AC_MSG_WARN([libavformat not found. building without MPEG support.])
fi

AC_OUTPUT
