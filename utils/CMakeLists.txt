#================================
# Define sources
#================================
SET( VTK_MNI_SRC
    vtkXFMReader.cxx
    vtkXFMWriter.cxx
    stringtools.cpp )

SET( VTK_MNI_HDR
    vtkXFMReader.h
    vtkXFMWriter.h
    stringtools.h )

#================================
# Define output
#================================
add_library( vtkMNI ${VTK_MNI_SRC} ${VTK_MNI_HDR} )

target_link_libraries( vtkMNI ${VTK_LIBRARIES} )
target_include_directories( vtkMNI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )