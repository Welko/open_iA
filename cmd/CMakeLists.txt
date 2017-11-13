SET (EXECUTABLE_NAME open_iA_cmd)

# open_iA Version number
include(GetGitRevisionDescription)
git_describe(VERSION --tags)
configure_file("${open_iA_SOURCE_DIR}/cmake/version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/version.h" @ONLY)

# Generate executable:
SET (MAIN_SOURCES "${CMAKE_SOURCE_DIR}/cmd/main.cpp")
ADD_EXECUTABLE( ${EXECUTABLE_NAME} ${MAIN_SOURCES} )

TARGET_LINK_LIBRARIES(${EXECUTABLE_NAME} PRIVATE ${CORE_LIBRARY_NAME})
TARGET_INCLUDE_DIRECTORIES(${EXECUTABLE_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}) # for version.h

# Windows-specific configuration
IF (MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")

	# Set up debugging/running environments	in Visual Studio to point to the correct dll files:
	STRING(REGEX REPLACE "/" "\\\\" VTK_WIN_DIR ${VTK_DIR})
	STRING(REGEX REPLACE "/" "\\\\" ITK_WIN_DIR ${ITK_DIR})
	STRING(REGEX REPLACE "/lib/cmake/Qt5" "" Qt5_WIN_DIR ${Qt5_DIR})
	STRING(REGEX REPLACE "/" "\\\\" Qt5_WIN_DIR ${Qt5_WIN_DIR})
	SET (RelWithDebInfoLibraryPaths "${VTK_WIN_DIR}\\bin\\RelWithDebInfo;${ITK_WIN_DIR}\\bin\\RelWithDebInfo;${Qt5_WIN_DIR}\\bin")
	SET (ReleaseLibraryPaths "${VTK_WIN_DIR}\\bin\\Release;${ITK_WIN_DIR}\\bin\\Release;${Qt5_WIN_DIR}\\bin")
	SET (DebugLibraryPaths "${VTK_WIN_DIR}\\bin\\Debug;${ITK_WIN_DIR}\\bin\\Debug;${Qt5_WIN_DIR}\\bin")
	SET (MinSizeRelLibraryPaths "${VTK_WIN_DIR}\\bin\\MinSizeRel;${ITK_WIN_DIR}\\bin\\MinSizeRel;${Qt5_WIN_DIR}\\bin")

	IF (OPENCL_FOUND)
		IF (EXISTS "${OPENCL_DLL}")
			STRING(REGEX REPLACE "/OpenCL.dll" "" OPENCL_WIN_DIR ${OPENCL_DLL})
			STRING(REGEX REPLACE "/" "\\\\" OPENCL_WIN_DIR ${OPENCL_WIN_DIR})
			SET (RelWithDebInfoLibraryPaths "${OPENCL_WIN_DIR};${RelWithDebInfoLibraryPaths}")
			SET (ReleaseLibraryPaths "${OPENCL_WIN_DIR};${ReleaseLibraryPaths}")
			SET (DebugLibraryPaths "${OPENCL_WIN_DIR};${DebugLibraryPaths}")
			SET (MinSizeRelLibraryPaths "${OPENCL_WIN_DIR};${MinSizeRelLibraryPaths}")
		ENDIF()
	ENDIF()

	IF (CUDA_FOUND)
		STRING(REGEX REPLACE "/" "\\\\" CUDA_WIN_DIR ${CUDA_TOOLKIT_ROOT_DIR})
		SET (RelWithDebInfoLibraryPaths "${CUDA_WIN_DIR}/bin;${RelWithDebInfoLibraryPaths}")
		SET (ReleaseLibraryPaths "${CUDA_WIN_DIR}/bin;${ReleaseLibraryPaths}")
		SET (DebugLibraryPaths "${CUDA_WIN_DIR}/bin;${DebugLibraryPaths}")
		SET (MinSizeRelLibraryPaths "${CUDA_WIN_DIR}/bin;${MinSizeRelLibraryPaths}")
	ENDIF()

	IF (ITK_USE_SYSTEM_FFTW)
		SET (RelWithDebInfoLibraryPaths "${ITK_FFTW_LIBDIR};${RelWithDebInfoLibraryPaths}")
		SET (ReleaseLibraryPaths "${ITK_FFTW_LIBDIR};${ReleaseLibraryPaths}")
		SET (DebugLibraryPaths "${ITK_FFTW_LIBDIR};${DebugLibraryPaths}")
		SET (MinSizeRelLibraryPaths "${ITK_FFTW_LIBDIR};${MinSizeRelLibraryPaths}")
	ENDIF(ITK_USE_SYSTEM_FFTW)

	IF (ASTRA_TOOLBOX_FOUND)
		IF (ASTRA_TOOLBOX_LIBRARIES_DEBUG)
			STRING(FIND ${ASTRA_TOOLBOX_LIBRARIES_DEBUG} "/" ASTRA_LIB_DEBUG_LASTSLASHPOS REVERSE)
			STRING(SUBSTRING ${ASTRA_TOOLBOX_LIBRARIES_DEBUG} 0 ${ASTRA_LIB_DEBUG_LASTSLASHPOS} ASTRA_LIBRARIES_DEBUG_PATH)
			STRING(REGEX REPLACE "/" "\\\\" ASTRA_LIBRARIES_DEBUG_WIN_PATH ${ASTRA_LIBRARIES_DEBUG_PATH})
			SET (DebugLibraryPaths "${ASTRA_LIBRARIES_DEBUG_WIN_PATH};${DebugLibraryPaths}")
		ENDIF()
		STRING(FIND ${ASTRA_TOOLBOX_LIBRARIES_RELEASE} "/" ASTRA_RELEASE_LIB_LASTSLASHPOS REVERSE)
		STRING(SUBSTRING ${ASTRA_TOOLBOX_LIBRARIES_RELEASE} 0 ${ASTRA_RELEASE_LIB_LASTSLASHPOS} ASTRA_LIBRARIES_RELEASE_PATH)
		STRING(REGEX REPLACE "/" "\\\\" ASTRA_LIBRARIES_RELEASE_WIN_PATH ${ASTRA_LIBRARIES_RELEASE_PATH})
		SET (RelWithDebInfoLibraryPaths "${ASTRA_LIBRARIES_RELEASE_WIN_PATH};${RelWithDebInfoLibraryPaths}")
		SET (ReleaseLibraryPaths "${ASTRA_LIBRARIES_RELEASE_WIN_PATH};${ReleaseLibraryPaths}")
		SET (MinSizeRelLibraryPaths "${ASTRA_LIBRARIES_RELEASE_WIN_PATH};${MinSizeRelLibraryPaths}")
	ENDIF()

	IF (NOT "${ITKZLIB_LIBRARIES}" STREQUAL "itkzlib")
		STRING(FIND "${ITKZLIB_LIBRARIES}" ";" SEMICOLONPOS)
		IF (SEMICOLONPOS EQUAL -1)
			SET(ZLIB_LIBRARY_RELEASE "${ITKZLIB_LIBRARIES}")
			SET(ZLIB_LIBRARY_DEBUG "${ITKZLIB_LIBRARIES}")
		ELSE()
			LIST (GET ITKZLIB_LIBRARIES 1 ZLIB_LIBRARY_RELEASE)
			LIST (GET ITKZLIB_LIBRARIES 3 ZLIB_LIBRARY_DEBUG)
		ENDIF()
		STRING(FIND ${ZLIB_LIBRARY_RELEASE} "/" ZLIBRELLIB_LASTSLASHPOS REVERSE)
		STRING(SUBSTRING ${ZLIB_LIBRARY_RELEASE} 0 ${ZLIBRELLIB_LASTSLASHPOS} ZLIB_REL_LIB_DIR)
		STRING(FIND ${ZLIB_LIBRARY_DEBUG} "/" ZLIBDBGLIB_LASTSLASHPOS REVERSE)
		STRING(SUBSTRING ${ZLIB_LIBRARY_DEBUG} 0 ${ZLIBDBGLIB_LASTSLASHPOS} ZLIB_DBG_LIB_DIR)
		MESSAGE(STATUS "ITK was built with system zlib, adding paths to dll. Release: ${ZLIB_REL_LIB_DIR}, Debug: ${ZLIB_DBG_LIB_DIR}")
		SET (RelWithDebInfoLibraryPaths "${ZLIB_REL_LIB_DIR};${RelWithDebInfoLibraryPaths}")
		SET (ReleaseLibraryPaths "${ZLIB_REL_LIB_DIR};${ReleaseLibraryPaths}")
		SET (DebugLibraryPaths "${ZLIB_DBG_LIB_DIR};${DebugLibraryPaths}")
		SET (MinSizeRelLibraryPaths "${ZLIB_REL_LIB_DIR};${MinSizeRelLibraryPaths}")
	ENDIF()
	
	IF (HDF5_FOUND)
		STRING(REGEX REPLACE "/cmake" "" HDF5_BASE_DIR ${HDF5_DIR})
		STRING(REGEX REPLACE "/" "\\\\" HDF5_BASE_DIR ${HDF5_BASE_DIR})
		IF (EXISTS "${HDF5_BASE_DIR}\\bin\\Release")
			SET (RelWithDebInfoLibraryPaths "${HDF5_BASE_DIR}\\bin\\RelWithDebInfo;${RelWithDebInfoLibraryPaths}")
			SET (ReleaseLibraryPaths "${HDF5_BASE_DIR}\\bin\\Release;${ReleaseLibraryPaths}")
			SET (DebugLibraryPaths "${HDF5_BASE_DIR}\\bin\\Debug;${DebugLibraryPaths}")
			SET (MinSizeRelLibraryPaths "${HDF5_BASE_DIR}\\bin\\MinSizeRel;${MinSizeRelLibraryPaths}")
		ELSE()
			SET (RelWithDebInfoLibraryPaths "${HDF5_BASE_DIR}\\bin;${RelWithDebInfoLibraryPaths}")
			SET (ReleaseLibraryPaths "${HDF5_BASE_DIR}\\bin;${ReleaseLibraryPaths}")
			SET (DebugLibraryPaths "${HDF5_BASE_DIR}\\bin;${DebugLibraryPaths}")
			SET (MinSizeRelLibraryPaths "${HDF5_BASE_DIR}\\bin;${MinSizeRelLibraryPaths}")
		ENDIF()
	ENDIF()

	IF (NOT EXISTS ${CMAKE_BINARY_DIR}/cmd/open_iA_cmd.vcxproj.user)	# find a way to update this if required
		CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmd/open_iA_cmd.vcxproj.user ${CMAKE_BINARY_DIR}/cmd @ONLY)
	ENDIF ()
ENDIF (MSVC)

IF (CMAKE_COMPILER_IS_GNUCXX)
	SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE")
	SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE")
ENDIF()

# Installation
INSTALL (TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION .)