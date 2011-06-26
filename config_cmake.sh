#!/bin/sh

opt=

# ON or OFF
opt="$opt -DPYTHON_BUILD_EXT=OFF"

# ON or OFF
opt="$opt -DPYTHON_INSTALL_EXT=OFF"

# ON or OFF
opt="$opt -DPYTHON_BUILD_EXT_INSTALLER=OFF"

# Release or Debug
opt="$opt -DCMAKE_BUILD_TYPE=Release"

# ON or OFF
opt="$opt -DUSE_IJG_CODEC=ON"

# ON or OFF
opt="$opt -DUSE_OPJ_CODEC=ON"

# ON or OFF
opt="$opt -DUSE_DISPLAY_DEBUGINFO=OFF"

echo OPTIONS ARE = $opt

#opt="$opt -D PYTHON_INCLUDE_DIR:STRING=/opt/python26/include/python2.6"
#opt="$opt -D PYTHON_LIBRARY:STRING=/opt/python26/lib/libpython2.6.so"

pypath="/System/Library/Frameworks/Python.framework/Versions/2.6"
opt="$opt -D PYTHON_INCLUDE_DIR:STRING=${pypath}/include/python2.6"
opt="$opt -D PYTHON_LIBRARY:STRING=${pypath}/Python"
opt="$opt -D PYTHON_EXECUTABLE:STRING=/usr/bin/python"

cmake -G"Unix Makefiles"  $opt .
