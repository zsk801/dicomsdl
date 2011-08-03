SET (C_CXX_SOURCES
	xmlparse.c
	xmlrole.c
	xmltok.c
	xmltok_impl.c
	xmltok_ns.c
)

configure_file(expat_config.h.cmakein
               expat_config.h @ONLY)
