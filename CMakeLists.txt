CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

PROJECT( GPURigidRegistration )

#----------------------------------------
# Tell the compiler to use c++11 which is required with Qt >= 5.7
set( CMAKE_CXX_STANDARD 11 )
#----------------------------------------

#==================================================================
# Look for VTK (required)
#==================================================================
set( AutoVtkPath ${CMAKE_CURRENT_BINARY_DIR} ) # dummy autopath to be changed
set( VTK_LONG_VERSION "8.1.2" ) # need to be changed

find_package(VTK ${VTK_LONG_VERSION} EXACT COMPONENTS vtkGUISupportQt vtkRenderingOpenGL vtkRenderingVolumeOpenGL vtkRenderingImage
    vtkRenderingAnnotation vtkImagingStencil vtkImagingStatistics vtkFiltersTexture
    vtkInteractionStyle vtkInteractionWidgets
    vtkIOXML vtkIOLegacy vtkIOPLY NO_MODULE
    PATHS ${AutoVtkPath} )
include(${VTK_USE_FILE})

#==================================================================
# Look for ITK (required)
#==================================================================
set( AutoItkPath ${CMAKE_CURRENT_BINARY_DIR} ) # dummy autopath
set( ITK_LONG_VERSION "4.13.3" )

find_package( ITK ${ITK_LONG_VERSION} EXACT REQUIRED PATHS ${AutoItkPath} )
include( ${ITK_USE_FILE} )

#==================================================================
# Look for Elastix (required by some plugins, but not the main app)
#==================================================================
set( AutoElastixPath ${CMAKE_CURRENT_BINARY_DIR} ) # dummy autopath
find_package( Elastix ${IBIS_ELASTIX_LONG_VERSION} REQUIRED EXACT PATHS ${AutoElastixPath} )
include( ${ELASTIX_USE_FILE} )
# Elastix 4.9 does not seem to include CMAEvolutionStrategy directory, we need to do it manually
include_directories( ${Elastix_DIR}/../src/Components/Optimizers/CMAEvolutionStrategy )





SET( ITK_REGISTRATION_OPENCL_SRC
    itkGPUGradientOrientationMatching.hxx
    itkGPU3DRigidSimilarityMetric.h
)

SET( ITK_REGISTRATION_OPENCL_HDR
    itkGPUGradientOrientationMatching.h
)

set( LibCL GPUDiscreteGaussianGradientImageFilter 
    GPUGradientOrientationMatchingKernel
)

set( SRC_FILES
        gpu_rigidregistration.cpp
    )
set( HEADER_FILES 
        gpu_rigidregistration.h
    )

#================================
# Create custom commands to
# encode each cl file into a
# C string literal in a header
# file.
#================================
foreach( file ${LibCL} )
  set( clKernel_src ${CMAKE_CURRENT_SOURCE_DIR}/${file}.cl )
  set( clKernel_cxx ${CMAKE_CURRENT_BINARY_DIR}/${file}.cxx )
  set( clKernel_h ${CMAKE_CURRENT_BINARY_DIR}/${file}.h )
  add_custom_command(
    OUTPUT ${clKernel_cxx} ${clKernel_h}
    DEPENDS ${clKernel_src}
    COMMAND ${VTK_ENCODESTRING_EXE}
    ARGS ${clKernel_cxx} ${clKernel_src} ${file} --build-header " " vtkSystemIncludes.h )
  set( LibCLSrc ${LibCLSrc} ${clKernel_src} ${clKernel_h} ${clKernel_cxx} )
  set( LibCLHdr ${LibCLHdr} ${clKernel_h} )
endforeach(file)

#================================
# Define output
#================================

#target_link_libraries( itkRegistrationOpenCL ${ITK_LIBRARIES}  ${OPENCL_LIBRARIES})

#================================
# Define include dir for
# dependent projects
#================================
target_include_directories( itkRegistrationOpenCL PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR} ${OPENCL_INCLUDE_DIRS} )


## need to check if all dependencies are found
# IF( NOT OPENCL_FOUND )
  # message( SEND_ERROR "OpenCL has not been found. This library is mandatory for the GPU Rigid Registration Plugin to build.\n" )
# ENDIF()

# IF( NOT ITK_USE_GPU )
  # message( SEND_ERROR "GPU_RigidRegistration plugin cannot build if ITK was built without ITK_USE_GPU enabled.\n" )
# ENDIF()

# IF( NOT IBIS_USE_ELASTIX )
  # message( SEND_ERROR "GPU_RigidRegistration plugin require Elastix library. Please enable IBIS_USE_ELASTIX.\n" )
# ENDIF()


#add library 
target_link_libraries( ${SERC_FILES} ${HEADER_FILES} ${ITK_REGISTRATION_OPENCL_SRC} ${ITK_REGISTRATION_OPENCL_HDR} ${LibCLSrc} ${LibCLHdr} )
