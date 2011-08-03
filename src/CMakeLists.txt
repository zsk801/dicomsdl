#  -----------------------------------------------------------------------
#
#  $Id$
# 
#  Copyright 2010, Kim, Tae-Sung. All rights reserved.
#  See copyright.txt for details
#
#  -----------------------------------------------------------------------

cmake_minimum_required (VERSION 2.8)

project (dicomsdl)

# ------------------------------------------------------------------------

include(TestBigEndian)
include(CheckIncludeFile)
include(CheckIncludeFiles)

test_big_endian(BIG_ENDIAN_MACHINE)
check_include_file(basetsd.h _HAVE_BASETSD_H)
check_include_file(stdint.h _HAVE_STDINT_H)
check_include_file(windows.h _HAVE_WINDOWS_H)
check_include_file(dirent.h _HAVE_DIRENT_H)


include_directories("${PROJECT_SOURCE_DIR}/lib"
					"${PROJECT_SOURCE_DIR}/ext"
					"${PROJECT_SOURCE_DIR}/ext/zlib"
					"${PROJECT_SOURCE_DIR}/ext/expat"
					)
	
# ------------------------------------------------------------------------
# OS specific compiler flags

IF(MSVC)
	# Suppressed warnings:
	# C4250: 'class1' : inherits 'class2::member' via dominance
	# C4355: 'this' : used in base member initializer list
	# C4503: 'function' : decorated name length exceeded, name was truncated
	# C4800: 'type' : forcing value to bool 'true' or 'false'
	# C4996: 'function': was declared deprecated

	SET(CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} /wd4250 /wd4355 /wd4503 /wd4800 /wd4996 -DWIN32 -DXML_STATIC -DHAVE_EXPAT_CONFIG_H")
	SET(CMAKE_C_FLAGS
		"${CMAKE_C_FLAGS} -wd4996 -wd4819 -wd4018 -wd4101 -wd4013 -DWIN32 -DXML_STATIC -DHAVE_EXPAT_CONFIG_H")
	 
	IF (BUILD_SHARED_LIBS)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D _BUILD_DLL")
	ENDIF(BUILD_SHARED_LIBS)
	
ELSE(MSVC)
	include(CheckCXXCompilerFlag)
	CHECK_CXX_COMPILER_FLAG("-fPIC" HAVE_FPIC_FLAG)
	IF(HAVE_FPIC_FLAG)
		SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -DXML_STATIC -DHAVE_EXPAT_CONFIG_H")
		SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -DXML_STATIC -DHAVE_EXPAT_CONFIG_H")
	ENDIF(HAVE_FPIC_FLAG)
	
ENDIF(MSVC)


# ------------------------------------------------------------------------
# setting C, CXX sources

MACRO (ADD_SOURCES SUBDIRNAME)
	SET (C_CXX_SOURCES )
	ADD_SUBDIRECTORY(${SUBDIRNAME})

	GET_DIRECTORY_PROPERTY(
		SUB_SOURCES DIRECTORY ${SUBDIRNAME}
		DEFINITION C_CXX_SOURCES
	)
	FOREACH (FN ${SUB_SOURCES})
		SET (C_CXX_SOURCES ${C_CXX_SOURCES} ${SUBDIRNAME}/${FN})
	ENDFOREACH(FN)

	GET_DIRECTORY_PROPERTY(
		EXT_SOURCES DIRECTORY ${SUBDIRNAME}
		DEFINITION EXT_C_CXX_SOURCES
	)
	FOREACH (FN ${EXT_SOURCES})
		STRING(REGEX REPLACE "\\\\" "/" FN ${FN})
		SET (C_CXX_SOURCES ${C_CXX_SOURCES} ${FN})
	ENDFOREACH(FN)

	GET_DIRECTORY_PROPERTY(
		ADD_CMAKE_CXX_FLAGS DIRECTORY ${SUBDIRNAME}
		DEFINITION ADD_CMAKE_CXX_FLAGS
	)
	FOREACH (FLAG ${ADD_CMAKE_CXX_FLAGS})
		SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
	ENDFOREACH(FLAG)

	GET_DIRECTORY_PROPERTY(
		ADD_CMAKE_C_FLAGS DIRECTORY ${SUBDIRNAME}
		DEFINITION ADD_CMAKE_C_FLAGS
	)
	FOREACH (FLAG ${ADD_CMAKE_C_FLAGS})
		SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
	ENDFOREACH(FLAG)
ENDMACRO (ADD_SOURCES)


# ------------------------------------------------------------------------
# Add C, CXX sources

SET (SOURCES )

# openjpeg -------------------------------------------------------
IF (USE_OPJ_CODEC)
	SET(ADD_CMAKE_CXX_FLAGS "-DOPJ_STATIC")
	SET(ADD_CMAKE_C_FLAGS "-DOPJ_STATIC")
	ADD_SOURCES(ext/opj)
	SET (SOURCES ${SOURCES} ${C_CXX_SOURCES})
ENDIF (USE_OPJ_CODEC)

# IJG library ----------------------------------------------------
IF (USE_IJG_CODEC)
	ADD_SOURCES(ext/ijg)
	SET (SOURCES ${SOURCES} ${C_CXX_SOURCES})
ENDIF (USE_IJG_CODEC)

# IPP library ----------------------------------------------------
IF (USE_IPP_CODEC)
	ADD_SOURCES(ext/ipp)
	SET (SOURCES ${SOURCES} ${C_CXX_SOURCES})
	
	GET_DIRECTORY_PROPERTY(IPP_INCLUDE_DIRECTORIES
		DIRECTORY ext/ipp
		DEFINITION IPP_INCLUDE_DIRECTORIES)
	include_directories(
		${IPP_INCLUDE_DIRECTORIES}
		${IPP_INCLUDE}
	)
	
	FIND_LIBRARY(ippcore_l	ippcore_l	PATH "${IPP_LIBPATH}")
	FIND_LIBRARY(ippi_l		ippi_l		PATH "${IPP_LIBPATH}")
	FIND_LIBRARY(ipps_l		ipps_l		PATH "${IPP_LIBPATH}")
	FIND_LIBRARY(ippcc_l	ippcc_l		PATH "${IPP_LIBPATH}")
	FIND_LIBRARY(ippj_l		ippj_l		PATH "${IPP_LIBPATH}")
	SET (EXT_LIBRARIES
		${ippcore_l} ${ippi_l} ${ipps_l} ${ippcc_l} ${ippj_l})
	
ENDIF (USE_IPP_CODEC)

# zlib -----------------------------------------------------------
IF (USE_ZLIB)
	ADD_SOURCES(ext/zlib)
	SET (SOURCES ${SOURCES} ${C_CXX_SOURCES})
ENDIF (USE_ZLIB)

# minizip --------------------------------------------------------
IF (USE_MINIZIP)
	ADD_SOURCES(ext/minizip)
	SET (SOURCES ${SOURCES} ${C_CXX_SOURCES})
ENDIF (USE_MINIZIP)

# expat ----------------------------------------------------------
IF (USE_EXPAT)
	ADD_SOURCES(ext/expat)
	SET (SOURCES ${SOURCES} ${C_CXX_SOURCES})
ENDIF (USE_EXPAT)

# dicomsdl -------------------------------------------------------

ADD_SOURCES(lib)
SET (SOURCES ${SOURCES} ${C_CXX_SOURCES})

# ------------------------------------------------------------------------
# Build and install

IF (BUILD_SHARED_LIBS)
	add_library(dicomsdl SHARED ${SOURCES})
ELSE (BUILD_SHARED_LIBS)
	add_library(dicomsdl ${SOURCES})
ENDIF (BUILD_SHARED_LIBS)

target_link_libraries (dicomsdl ${EXT_LIBRARIES})

INSTALL (TARGETS dicomsdl
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

# ------------------------------------------------------------------------
# Build Wrapper

IF (PYTHON_BUILD_EXT)
	ADD_SUBDIRECTORY(wrapper)
ENDIF (PYTHON_BUILD_EXT)

# ------------------------------------------------------------------------
# Build Examples

ADD_SUBDIRECTORY(example)