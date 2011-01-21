@echo off
set OPTION=

REM ON or OFF
set OPTION=%OPTION% -DPYTHON_BUILD_EXT=OFF

REM ON or OFF
set OPTION=%OPTION% -DPYTHON_INSTALL_EXT=OFF

REM ON or OFF
set OPTION=%OPTION% -DPYTHON_BUILD_EXT_INSTALLER=OFF

REM Release or Debug
set OPTION=%OPTION% -DCMAKE_BUILD_TYPE=Release

REM ON or OFF
set OPTION=%OPTION% -DUSE_IJG_CODEC=ON

REM ON or OFF
set OPTION=%OPTION% -DUSE_OPJ_CODEC=ON

REM ON or OFF
set OPTION=%OPTION% -DUSE_IPP_CODEC=OFF
set OPTION=%OPTION% -DIPP_INCLUDE:string="c:\Program Files (x86)\Intel\ComposerXE-2011\ipp\include"
set OPTION=%OPTION% -DIPP_LIBPATH:string="c:\Program Files (x86)\Intel\ComposerXE-2011\ipp\lib\ia32"
set OPTION=%OPTION% -DIPP_SAMPLEROOT:string="C:\Lab\workspace\ipp-samples"
set OPTION=%OPTION% -DIPP_UICBINPATH:string="C:\Lab\workspace\ipp-samples\image-codecs\uic\_bin\ia32_cl9"

REM ON or OFF
set OPTION=%OPTION% -DUSE_DISPLAY_DEBUGINFO=OFF

REM ON or OFF
set OPTION=%OPTION% -DBUILD_SHARED_LIBS=OFF

echo OPTIONS ARE =%OPTION%

cmake -G"NMake Makefiles" %OPTION% .