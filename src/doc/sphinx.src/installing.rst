Installing
==========

Getting Software
----------------

You can get the source code, binaries and installer for python at the
http://code.google.com/p/dicomsdl .

Installing
----------

DICOM SDL needs cmake (www.cmake.org) to compile. DICOM SDL was successfully
compiled and ran on following environments

   * Microsoft Windows x86/x64 using Microsoft visual studio 2008,
   * Linux x64 (CentOS) using gcc,
   * MacOS (ppc) using xcode 3.1.2

DICOM SDL is also expected to be compiled successfully at other Linux x86/x64
and intel MacOS platforms.

Compiling
---------

Using VisualStudio C++ compilers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

General command lines for build libraries are like this

::

   command prompt> cd [DICOMSDL root directory]
   ....\dicomsdl> cmake -G"NMake Makefiles" [OPTION] .
   ....\dicomsdl> nmake install

.. option:: -DCMAKE_BUILD_TYPE=[Release|Debug]

   Build `Release` or `Debug` mode library.
   
.. option:: -DBUILD_SHARED_LIBS=ON

   Build a shared library.
   
.. option:: -DBUILD_PYTHON_EXT=ON

   Build python extension. :program:`python` and :program:`swig.exe`
   should be installed. :program:`swig.exe` should be found in command line path. 

Build a static DICOMSDL library.

::

   command prompt> cmake -G"NMake Makefiles" -DCMAKE_BUILD_TYPE=Release .
   command prompt> nmake install
   

Build a python module. 

::

   command prompt> cmake -G"NMake Makefiles" -DBUILD_PYTHON_EXT=ON -DCMAKE_BUILD_TYPE=Release .
   command prompt> nmake install

Build a shared DICOMSDL library 

:: 

   command prompt> cmake -G"NMake Makefiles" -DBUILD_SHARED_LIBS=ON  .
   command prompt> nmake install

Compiled binaries will go into :file:`.\build` directory. Includes
:file:`dicom.h`, :file:`dicomcfg.h` and :file:`dicomsdk.lib` for your project
and add line ``#include "dicom.h"`` into your program.