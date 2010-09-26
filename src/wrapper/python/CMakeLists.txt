SET(BUILD_SHARED_LIBS OFF)

IF(_HAVE_BASETSD_H)
	SET(INCLUDE_TYPEMAP	"windows.i")
ENDIF(_HAVE_BASETSD_H)	

IF(_HAVE_STDINT_H)
	SET(INCLUDE_TYPEMAP	"stdint.i")
ENDIF(_HAVE_STDINT_H)

configure_file (
  "dicom.i.in"
  "dicom.i"
)

FIND_PACKAGE(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})
FIND_PACKAGE(PythonInterp) 
FIND_PACKAGE(PythonLibs)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
SET(CMAKE_SWIG_FLAGS "")

SET_SOURCE_FILES_PROPERTIES(dicom.i PROPERTIES CPLUSPLUS ON)
SWIG_ADD_MODULE(dicom python dicom.i)
SWIG_LINK_LIBRARIES(dicom ${PYTHON_LIBRARIES} dicomsdl)

IF (WIN32)
SET (PYEXT_FILES dicom.py _dicom.pyd)
ELSE (WIN32)
SET (PYEXT_FILES dicom.py _dicom.so)
ENDIF (WIN32)

add_custom_command (TARGET test POST_BUILD COMMAND echo -R
WORKING_DIRECTORY . COMMENT "Creating tags" VERBATIM)

INSTALL (FILES ${PYEXT_FILES} DESTINATION bin)

IF (BUILD_PYTHON_EXT_INSTALLER)
	# Creating python module installer
	add_executable (dummy_proj dummy.c)

	IF (WIN32)
		add_custom_command (TARGET dummy_proj POST_BUILD
			COMMAND ${PYTHON_EXECUTABLE} setup.py build bdist_wininst
			COMMENT "Creating python module installer" VERBATIM)
	ELSE (WIN32)
		add_custom_command (TARGET dummy_proj POST_BUILD
			COMMAND ${PYTHON_EXECUTABLE} setup.py build bdist_rpm
			COMMENT "Creating python module installer" VERBATIM)
	ENDIF (WIN32)
	
ENDIF (BUILD_PYTHON_EXT_INSTALLER)