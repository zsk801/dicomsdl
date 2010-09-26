#!/bin/sh

opt=

# ON or OFF
opt="$opt -DBUILD_PYTHON_EXT=ON"

# ON or OFF
opt="$opt -DBUILD_PYTHON_EXT_INSTALLER=ON"

# Release or Debug
opt="$opt -DCMAKE_BUILD_TYPE=Release"

# ON or OFF
opt="$opt -DUSE_IJG_CODEC=OFF"

# ON or OFF
opt="$opt -DUSE_OPJ_CODEC=OFF"

# ON or OFF
opt="$opt -DUSE_DISPLAY_DEBUGINFO=OFF"

echo OPTIONS ARE = $opt

opt="$opt -D PYTHON_INCLUDE_DIR:STRING=/opt/python26/include/python2.6"
opt="$opt -D PYTHON_LIBRARY:STRING=/opt/python26/lib/libpython2.6.so"
cmake -G"Unix Makefiles"  $opt .
