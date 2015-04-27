file (GLOB EasyBMP_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

#message( "adding easybmp" )
#message( "${EasyBMP_SOURCE_FILE}")

#set (SOURCE_FILE ${SOURCE_FILE} ${EasyBMP_SOURCE_FILE} CACHE INTERNAL "list for source files")
#message( " easy bmp: ${SOURCE_FILE}")
set (EasyBMP_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "directory to EasyBMP headers")
add_library( EasyBMP ${EasyBMP_SOURCE_FILE})