cmake_minimum_required (VERSION 2.6)
project (bwProject)
file (GLOB CURRENT_SOURCE_FILE source/*.cxx)

set ( SOURCE_FILE ${CURRENT_SOURCE_FILE} CACHE INTERNAL "list for source files" )

find_package(MPI REQUIRED) #make it REQUIRED, if you want
include_directories(SYSTEM ${MPI_INCLUDE_PATH})
set(LINK_LIBRARIES "${MPI_C_LIBRARIES}")

set(USE_EasyBMP 1)
set(USE_SDF 1)
set(USE_WaveletAnalysis 1)

# Add EasyBMP for image output
add_subdirectory( Modules )

#FIND_PACKAGE( OpenMP REQUIRED)
#if(OPENMP_FOUND)
#	message("OPENMP FOUND")
#	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
#endif()

#find_package( VTK COMPONENTS
#	vtkFiltersSources
#	vtkInteractionStyle
#	vtkRenderingOpenGL
#)
#include (${VTK_USE_FILE})
#set(VTK_DIR "/home/jie/study/cs595/VTK/VTK-build")

#message( ${SOURCE_FILE})

if (USE_EasyBMP)
	include_directories(${EasyBMP_INCLUDE})
	set(LINK_LIBRARIES ${LINK_LIBRARIES} EasyBMP)
endif (USE_EasyBMP)

if (USE_WaveletAnalysis)
	include_directories(${WaveletAnalysis_INCLUDE})
	set(LINK_LIBRARIES ${LINK_LIBRARIES} WaveletAnalysis) 
endif (USE_WaveletAnalysis)

add_executable(demo ${SOURCE_FILE})
#target_link_libraries(demo ${MPI_C_LIBRARIES})
target_link_libraries(demo ${LINK_LIBRARIES})


if (USE_SDF)
	set(SDF_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/ExternalLibraries/SDF/libSDF.a)
	add_custom_target(build_SDF ALL 
	                   COMMAND ${CMAKE_MAKE_PROGRAM}
	                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ExternalLibraries/SDF
	                   COMMENT "Original SDF makefile target")
	
	add_library(SDF STATIC IMPORTED)
	set_property(TARGET SDF APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
	set_target_properties(SDF PROPERTIES
							 IMPORTED_LOCATION_NOCONFIG "${SDF_LIBRARY}")
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ExternalLibraries/SDF)
	add_dependencies(SDF build_SDF)
	target_link_libraries(demo SDF)  
endif (USE_SDF)