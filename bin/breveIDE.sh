#!/bin/sh

#
# Apparently not all systems have the same version of Python,
# and no static version seems to be available.  The breve 
# distribution therefore includes a copy of the Python 2.4
# library.
#

DIRECTORY=`dirname $0`

export LD_LIBRARY_PATH=$DIRECTORY/lib:$LD_LIBRARY_PATH
export PYTHONHOME=$DIRECTORY

$DIRECTORY/breveIDE_ex $*
