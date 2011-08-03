SET (C_CXX_SOURCES
	opj_codec.cxx	
	libopenjpeg/bio.c
	libopenjpeg/cio.c
	libopenjpeg/dwt.c
	libopenjpeg/event.c
	libopenjpeg/image.c
	libopenjpeg/j2k.c
	libopenjpeg/j2k_lib.c
	libopenjpeg/jp2.c
	libopenjpeg/jpt.c
	libopenjpeg/mct.c
	libopenjpeg/mqc.c
	libopenjpeg/openjpeg.c
	libopenjpeg/pi.c
	libopenjpeg/raw.c
	libopenjpeg/t1.c
	libopenjpeg/t2.c
	libopenjpeg/tcd.c
	libopenjpeg/tgt.c
)

CONFIGURE_FILE(openjpeg_mangled.h libopenjpeg/openjpeg_mangled.h COPYONLY) 
CONFIGURE_FILE(openjpeg.h libopenjpeg/openjpeg.h COPYONLY) 
CONFIGURE_FILE(opj_config.h.in.user libopenjpeg/opj_config.h COPYONLY) 
