#!/bin/bash

function delete {
    find . -name $1 -exec rm -rvf {} +
}

function delete2 {
    find . -name $1 -exec echo {} +
}



delete CMakeCache.txt
delete CMakeFiles
delete Makefile
delete *.cmake
delete CTestTestfile.cmake
delete dicom.i
delete *.rst
delete zconf.h

delete *.a
delete _*.so

rm -vf src/doc/html
rm -vf src/doc/latex

rm -vf src/example/changetsuid
rm -vf src/example/dicomfiledump
rm -vf src/example/makedicomdir
rm -vf src/example/test
rm -vr src/example/example_getvalue
rm -vf src/wrapper/python/dummy_proj

rm -vf src/ext/ijg/8/*.c
rm -vf src/ext/ijg/8/*.h
rm -vf src/ext/ijg/12/*.c
rm -vf src/ext/ijg/12/*.h
rm -vf src/ext/ijg/16/*.c
rm -vf src/ext/ijg/16/*.h

rm -rvf build
rm -rvf src/wrapper/python/dist
rm -rvf src/wrapper/python/build
rm -vf src/wrapper/python/dicom_wrap.cpp
rm -vf src/wrapper/python/dicomPYTHON_wrap.cxx
delete dicom.py

delete install_manifest.txt
