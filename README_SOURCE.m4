
                  - breve source code release - version _VERSION_ -
                         http://www.spiderland.org/breve
                              jk [at] spiderland.org

                             -- INTRODUCTION --

breve is a free simulation environment designed for multi-agent simulation.  
breve allows users to define the behaviors of autonomous agents in a 
continuous 3D world, then observe how they interact.  breve includes support 
for a rich visualization engine, realistic physical simulation and an 
easy-to-use scripting language.

                           -- BUILDING BREVE --

1) Confirm that you have all of the required libraries installed.

  See the sections on "EXTERNAL LIBRARIES" below for more information
  on the libraries required to build breve.

2) Specify paths of external library and header files, if required

  If you have external libraries or headers installed in locations 
  other than /usr/{lib or include} or /usr/local/{lib or include}, 
  you may have to specify their location by setting the environment
  variables LDFLAGS and CPPFLAGS.

  Example: 

	export LDFLAGS='-L/opt/local/lib' CPPFLAGS='-I/opt/local/include'

3) ./configure

4) make optimize

To build the Mac OS X application:

5) make OSX_build

To build the wxWindows frontend for Linux and Windows:

5) cd wx; make

                             -- BUILD NOTES -- 

* For build notes on specific libraries that breve uses, please see the 
  "EXTERNAL LIBRARIES: DETAILS" section below.

* What are those errors about bin/osmesaloader.cc?  Those are harmless.
  osmesaloader is a completely optional extension which allows CLI 
  versions of breve to use features like light-exposure detection and 
  movie exporting.  It will only compile when Mesa libraries are found.

* OS X Note #1: if a library is installed, but cannot be found with the 
  configure script, you may have to run "ranlib" on the library before
  it can be found.  See the ranlib manpage for more information.

* OS X Note #2: the Xcode build targets breve.app is a simple shell script 
  wrapper to build real_breve.app using the correct libraries from the 
  configure script.  Building real_breve.app directly will fail, because 
  it is missing these library settings.

* For the record, I am well aware that parts of this source are a total mess.  
  This project and code, in one form or another has spanned much of my undergraduate 
  education, a Master's degree and several years in research and industry.  
  Parts of this code are now almost 10 years old.  Piece by piece I'm updating
  the code and moving it to C++.

                           --- PLATFORMS ---

breve is actively developed and maintained on three platforms:
	Mac OS X
	Linux (x86)
	MinGW (Windows)

breve has compiled successfully in the past on these other platforms 
as well, though some work may be required:
	Irix
	Solaris
	CYGWIN (Windows)
	Microsoft Visual C++

Please report successful ports and the changes required back to the author 
at jk [at] spiderland.org.

                    --- EXTERNAL LIBRARIES: SUMMARY ---

It's always a drag to have to download another set of libraries to get 
something working.  Sorry.  Below is a summary of libraries which breve
uses.  Some of these libraries are required, meaning that breve will not
build without them.  Others are optional, meaning that breve will build
without them, but that some features will not be available.

Many of these libraries are included with standard OS distributions and
developer tools and do not need to be downloaded separately.

Required:
  OpenGL
  GLUT
  ODE
  expat
  zlib
  libpthread
  libdl
  libgsl
  libgslcblas (or other BLAS library)

Optional:
  libpng
  libjpeg
  libportaudio
  libsndfile
  libffmpeg
  libpush
  libenet
  libqgame++

                    --- EXTERNAL LIBRARIES: DETAILS ---

+ [required] OpenGL, graphics libraries 
  Platform specific -- consult your OS vendor for more information.

+ [required] GLUT, OpenGL Utility Library
  http://www.opengl.org/resources/libraries/glut.html

  Included by default on many platforms.  Windows users can obtain the DLL from:
    http://www.xmission.com/~nate/glut.html.

+ [required] ODE, physical simulation library 
  http://www.ode.org

  Use version 0.9 or later and run configure with the following flags:

  --enable-double-precision --enable-release

  Be sure to compile with "dReal" as "double", as opposed to "float".

  Some versions of ODE make liberal use of alloca() to allocate memory 
  on the stack during simulation.  This can cause crashes if the simulation 
  becomes too complex and causes a stack overflow.  For this reason, official 
  builds of breve use a modified version of the ODE source.



+ [required for breveIDE build] wxwidgets, wx Interface Library
  http://www.wxwidgets.org

  This library provides the user interface functionality used by breveIDE
  on Linux and Windows.

  This library may need to be compiled with the flag --disable-unicode .

+ [required] libdl, dynamic loading library 

  This library is already installed on most platforms.  A notable 
  exception is when building on or for a version of Mac OS X prior to 
  10.3.  For pre-10.3 versions of Mac OS X, users will need to install
  a library called "dlcompat".

+ [required] expat, XML parsing library 
  http://expat.sourceforge.net/

+ [required] zlib, gzip compression library
  http://www.gzip.org/zlib/

+ [required] libgsl, GNU scientific library
  http://www.gnu.org

  Provides support for the Matrix and Random Number Generation classes.

+ [required] libgslblas, or other Basic Linear Algebra Subroutines library
  http://www.gnu.org

  A library of subroutines required by libgsl.  Distributed with GSL. 
  This library may be replaced by other implementations of the BLAS 
  library.

  On Mac OS X, the System vecLib framework is used, which provides 
  hardware vector acceleration on supported chips.



+ [optional] libjpeg, JPEG I/O library 
  http://www.ijg.org/

  Provides support for loading JPG images.  This library is optional.  
  If it is not available, JPG import will not be supported.

+ [optional] libpng, PNG I/O library 
  http://www.libpng.org/pub/png/libpng.html

  Provides support for loading and saving PNG images.  This library is 
  optional.  If it is not available, PNG import and export will not be 
  supported.

+ [optional] libportaudio, portable sound APIs
  http://www.portaudio.com/

  Provides support for playing soundfiles.  This library is optional.  
  If it is not available, sound support will not be available.

+ [optional] libsndfile, portable sound file I/O library 
  http://www.zip.com.au/~erikd/libsndfile/

  Provides support for loading soundfiles.  This library is optional.  
  If it is not available, sound support will not be available.

+ [optional] libffmpeg, mpeg movie export library
  http://ffmpeg.sourceforge.net/

  Provides support for exporting MPEG movies of simulations.  This library
  is optional.  If it is not available, movie export will not be supported.

  The ffmpeg maintainers do not typically make official release versions, 
  and the ffmpeg APIs do change from time to time.  Because of this, it can
  sometimes be difficult to get ffmpeg working with breve.

  Version 2.6 of breve is reported to compile correctly with ffmpeg subversion 
  revision 4709.

+ [optional] libpush, push language libary 
  http://push-evolve.sourceforge.net

  Provides support for the Push programming language for evolutionary
  computation.  This library is optional.  If it is not available, the 
  Push language will not be supported.  Requires GCC 3.3 or higher.

+ [optional] libenet, networking library 
  http://enet.cubik.org/

  Very optional.  Not yet used for any release-quality features.

+ [optional] libqgame++, Quantum Gate and Measurement Emulator
  http://hampshire.edu/lspector/qgame++

  Very optional.  A library for simulating quantum computing.

                                -- License --

/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000-2007 Jonathan Klein                                    *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program; if not, write to the Free Software               *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *****************************************************************************/
