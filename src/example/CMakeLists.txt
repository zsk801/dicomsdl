SET (EXAMPLE_SOURCES
	getting_started
	changetsuid
	dicomfiledump
	makedicomdir
	example_getvalue
	test
)

SET (PY_EXAMPLE_SOURCES
	makedicomdir.py
	dumpdicomdir.py
	example.py
	test.py
	modifyelement.py
)

FOREACH (FN ${EXAMPLE_SOURCES})
	add_executable (${FN} ${FN}.cxx)
	target_link_libraries (${FN} dicomsdl)
ENDFOREACH (FN)

INSTALL ( TARGETS ${EXAMPLE_SOURCES}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)
	
INSTALL (FILES ${PY_EXAMPLE_SOURCES} DESTINATION bin)
	