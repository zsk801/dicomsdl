#  -----------------------------------------------------------------------
#
#  $Id$
# 
#  Copyright 2010, Kim, Tae-Sung. All rights reserved.
#  See copyright.txt for details
#
#  -----------------------------------------------------------------------

cmake_minimum_required (VERSION 2.8)

SET (dicomsdl_version 0.79.20110728)

# ------------------------------------------------------------------------
# User options

#SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_INSTALL_PREFIX "./build")

OPTION(BUILD_SHARED_LIBS "Build dicomsdl with shared libraries." OFF)

OPTION(USE_DISPLAY_DEBUGINFO "Use display debugging information" OFF)
IF (USE_DISPLAY_DEBUGINFO)
	SET(__DEBUG__ ON)
ENDIF (USE_DISPLAY_DEBUGINFO)

OPTION(USE_OPJ_CODEC
		"Use openjpeg to decode/encode jpeg2000 images." ON)
OPTION(USE_IJG_CODEC
		"Use IJG library to decode/encode jpeg images." ON)
		
OPTION(USE_IPP_CODEC
		"Use ipp library to decode/encode jpeg/jpeg2000 images." OFF)
IF (USE_IPP_CODEC)
	FIND_PATH(IPP_INCLUDE "ippdefs.h")
	FIND_PATH(IPP_LIBPATH "ippcore_l.lib")
	FIND_PATH(IPP_SAMPLEROOT "ippEULA.rtf")
	FIND_PATH(IPP_UICBINPATH "bin/uic_jpeg-1.0.dll")
ENDIF (USE_IPP_CODEC)

OPTION(USE_LT_CODEC
		"Use leadtools to decode/encode jpeg/jpeg2000 images." OFF)
OPTION(USE_ZLIB
		"Use zlib to decode/encode deflated explicit LE images." ON)
OPTION(USE_MINIZIP
		"Use minizip to read dicomzip file." ON)
OPTION(USE_EXPAT
		"Use expat to convert DICOM file into/from xml format." ON)

OPTION(PYTHON_BUILD_EXT
		"Build Python Extension." OFF)
OPTION(PYTHON_INSTALL_EXT
		"Install Python Extension." OFF)
OPTION(PYTHON_BUILD_EXT_INSTALLER
		"Build Python Extension Installer." OFF)


# ------------------------------------------------------------------------

ADD_SUBDIRECTORY(src)