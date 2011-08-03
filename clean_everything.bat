del /s CMakeCache.txt
del /s *.ilk
del /s *.pdb
del /s *.cmake
del /s Makefile
del /s *.obj
del /s *.lib

del /s *.dll
del /s *.dll.manifest
del /s _*.pyd
del /s _*.pyd.manifest
del /s *.exe
del /s *.exe.manifest
del /s *.exp
del /s *.pyc
del /s dicom.i
del /s *.manifest
del /s *.manifest.res
del /s *.resource.txt
del /s *.rst
del /s *.CTestTestfile.cmake
del src\example\changetsuid
del src\example\dicomfiledump
del src\example\makedicomdir
del src\wrapper\python\dummy_proj
del src\wrapper\python\MANIFEST
del src\ext\zlib\zconf.h

del /s *.vcproj*
del /s *.sln
del /s *.suo
del /s *.ncb
del /s *.exp
del /s *.a
attrib -h *.suo
del /s *.suo
del /s lib*.so
del /s install_manifest.txt
del /s _*.so

rmdir /s/q CMakeFiles
rmdir /s/q CMakeFiles\CMakeTmp\CMakeFiles
rmdir /s/q src\CMakeFiles
rmdir /s/q src\example\CMakeFiles
rmdir /s/q src\ext\ijg\CMakeFiles
rmdir /s/q src\ext\ipp\CMakeFiles
rmdir /s/q src\ext\minizip\CMakeFiles
rmdir /s/q src\ext\opj\CMakeFiles
rmdir /s/q src\ext\zlib\CMakeFiles
rmdir /s/q src\lib\CMakeFiles
rmdir /s/q src\wrapper\CMakeFiles
rmdir /s/q src\wrapper\python\CMakeFiles
rmdir /s/q src\ext\expat\CMakeFiles
rmdir /s/q src\wrapper\python\dicomsdl-0.72
rmdir /s/q src\doc\html
rmdir /s/q src\doc\latex

del src\ext\ijg\8\*.c
del src\ext\ijg\8\*.h
del src\ext\ijg\12\*.c
del src\ext\ijg\12\*.h
del src\ext\ijg\16\*.c
del src\ext\ijg\16\*.h

rmdir /s/q build
rmdir /s/q src\wrapper\python\dist
rmdir /s/q src\wrapper\python\build
del src\wrapper\python\dicom_wrap.cpp
del src\wrapper\python\dicomPYTHON_wrap.cxx
del /s dicom.py