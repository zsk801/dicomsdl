
SET (SUBDIRS "")
MACRO (LIST_SUBDIRS basedir)
	FILE (GLOB subfiles "${basedir}/*")
	FOREACH (fn ${subfiles})
		IF (IS_DIRECTORY ${fn})
			MESSAGE("---- ipp-sample sub-directory ${fn}")
			SET (SUBDIRS ${SUBDIRS} ${fn})
			LIST_SUBDIRS(${fn})
		ENDIF ()
	ENDFOREACH (fn)
ENDMACRO()

STRING(REGEX REPLACE "\\\\" "/" IPP_SAMPLEROOT ${IPP_SAMPLEROOT})
LIST_SUBDIRS("${IPP_SAMPLEROOT}/image-codecs/uic/src/core")
LIST_SUBDIRS("${IPP_SAMPLEROOT}/image-codecs/uic/src/io")
LIST_SUBDIRS("${IPP_SAMPLEROOT}/image-codecs/uic/src/codec/image/jpeg2000")
SET (IPP_INCLUDE_DIRECTORIES ${SUBDIRS})

FILE (GLOB_RECURSE subfiles_core
		"${IPP_SAMPLEROOT}/image-codecs/uic/src/core/*.cpp")
FILE (GLOB_RECURSE subfiles_io
		"${IPP_SAMPLEROOT}/image-codecs/uic/src/io/*.cpp")
FILE (GLOB_RECURSE subfiles_jpeg2000
		"${IPP_SAMPLEROOT}/image-codecs/uic/src/codec/image/jpeg2000/*.cpp")
FILE (GLOB_RECURSE subfiles
		${subfiles_core} ${subfiles_io} ${subfiles_jpeg2000})

SET (EXT_C_CXX_SOURCES ${subfiles})


SET (C_CXX_SOURCES
	ipp_codec.cxx
)
