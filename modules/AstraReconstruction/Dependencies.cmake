SET( DEPENDENCIES_CMAKE
	ASTRA_TOOLBOX_FOUND
	CUDA_FOUND
)

SET( DEPENDENCIES_LIBRARIES_DEBUG
	${ASTRA_TOOLBOX_LIBRARIES_DEBUG}
)

SET( DEPENDENCIES_LIBRARIES_RELEASE
	${ASTRA_TOOLBOX_LIBRARIES_RELEASE}
)

SET( DEPENDENCIES_LIBRARIES
	${CUDA_CUDART_LIBRARY}
)

SET( DEPENDENCIES_INCLUDE_DIRS
	${ASTRA_TOOLBOX_INCLUDE_DIRS}
	${CUDA_TOOLKIT_INCLUDE}
)
