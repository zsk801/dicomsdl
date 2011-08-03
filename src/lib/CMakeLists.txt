#  -----------------------------------------------------------------------
#
#  $Id$
# 
#  Copyright 2010, Kim, Tae-Sung. All rights reserved.
#  See copyright.txt for details
#
#  -----------------------------------------------------------------------

# ------------------------------------------------------------------------
configure_file (
  "dicomcfg.h.in"
  "dicomcfg.h"
)

SET (HEADER_FILES
	dicom.h
	dicomcfg.h
	utilfunc.h
)

SET(C_CXX_SOURCES
	${C_CXX_SOURCES}
	dataelement.cxx
	dataset.cxx
	deflate.cxx
	dicomdict.cxx
	dicomdir.cxx
	dicomfile.cxx
	dicomfile_sorter.cxx
	errormsg.cxx
	imagecodec.cxx
	instream.cxx
	instreamobj.cxx
	memutil.cxx
	outstream.cxx
	pixelsequence.cxx
	recordoffset.cxx
	sequence.cxx
	zipfile.cxx
	uidutil.cxx
	fileutil.cxx
	dicomxml.cxx
)


# ------------------------------------------------------------------------

INSTALL (FILES ${HEADER_FILES} DESTINATION include)
