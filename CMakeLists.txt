CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

project( GPURigidRegistration )

#----------------------------------------
# Tell the compiler to use c++11
set( CMAKE_CXX_STANDARD 11 )
#----------------------------------------

#-----------------------------------------------------------------------------
# GIT - Let's check if a valid version of GIT is available
#-----------------------------------------------------------------------------
OPTION(USE_GIT_PROTOCOL "If behind a firewall turn this off to use https instead." OFF)

SET(GIT_PROTOCOL "git")
IF(NOT USE_GIT_PROTOCOL)
   SET(GIT_PROTOCOL "https")
ENDIF()

#==================================================================
# Look for VTK, only using vtkTransform, so any version should work
#==================================================================
find_package(VTK REQUIRED)

#==================================================================
# Look for ITK, version should be > 4.12
#==================================================================
find_package( ITK REQUIRED )
include( ${ITK_USE_FILE} )

#==================================================================
# Look for Elastix 
#==================================================================
find_package( Elastix REQUIRED )
include( ${ELASTIX_USE_FILE} )
# Elastix 4.9 does not seem to include CMAEvolutionStrategy directory, we need to do it manually
include_directories( ${Elastix_DIR}/../src/Components/Optimizers/CMAEvolutionStrategy )

#==================================================================
# Define sources
#==================================================================
set( CL_files 
        GPUDiscreteGaussianGradientImageFilter 
        GPUGradientOrientationMatchingKernel )

set( SRC_files
        gpu_rigidregistration.cpp )

set( HDR_files 
        gpu_rigidregistration.h
        itkGPUGradientOrientationMatching.hxx
        itkGPU3DRigidSimilarityMetric.h
        itkGPUGradientOrientationMatching.h )

#================================
# Create custom commands to
# encode each cl file into a
# C string literal in a header
# file.
#================================
# Find vtkEncodeString utility executable. Look in built or installed dir structure
if( ${VTK_MAJOR_VERSION} LESS "9" )

  include(${VTK_USE_FILE})
  set( vtkEncodeStringExeFile vtkEncodeString-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION} )
  find_program( VTK_ENCODESTRING_EXE ${vtkEncodeStringExeFile} PATHS
                ${VTK_DIR}/bin/
                ${VTK_DIR}/bin/Debug/
                ${VTK_DIR}/bin/Release/
                ${VTK_INSTALL_PREFIX}/bin
                NO_DEFAULT_PATH )
  if( NOT VTK_ENCODESTRING_EXE )
      message( FATAL_ERROR "the vtkEncodeString executable could not be found in the VTK directory.")
  endif()
  
  foreach( file ${CL_files} )
    set( clKernel_src ${CMAKE_CURRENT_SOURCE_DIR}/${file}.cl )
    set( clKernel_cxx ${CMAKE_CURRENT_BINARY_DIR}/${file}.cxx )
    set( clKernel_h ${CMAKE_CURRENT_BINARY_DIR}/${file}.h )
    add_custom_command(
      OUTPUT ${clKernel_cxx} ${clKernel_h}
      DEPENDS ${clKernel_src}
      COMMAND ${VTK_ENCODESTRING_EXE}
      ARGS ${clKernel_cxx} ${clKernel_src} ${file} --build-header " " vtkSystemIncludes.h )
    set( SRC_CL ${SRC_CL} ${clKernel_src} ${clKernel_h} ${clKernel_cxx} )
    set( HDR_CL ${HDR_CL} ${clKernel_h} )
  endforeach(file)
  
else()

  set( LibCLSrc )
  set( LibCLHdr )
  foreach (shader_file IN LISTS CL_files)
    vtk_encode_string(
      INPUT         "${shader_file}.cl"
      HEADER_OUTPUT clKernel_h
      SOURCE_OUTPUT clKernel_cxx)
    list(APPEND SRC_CL ${clKernel_cxx})
    list(APPEND HDR_CL ${clKernel_h})
  endforeach ()

endif()


#================================
# Define output
#================================
set( libName GPURigidRegistrationLib )
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library( ${libName} ${SRC_files} ${HDR_files} ${SRC_CL} ${HDR_CL} )
target_link_libraries( ${libName} ${ITK_LIBRARIES}  ${VTK_LIBRARIES} ${ELASTIX_LIBRARIES} )
target_include_directories( ${libName} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

#================================
# Options
#================================

option(BUILD_EXECUTABLE "Generate executable for GPURigidRegistration." ON)
option(BUILD_TEST "Enable testing for GPURigidRegistration.\n" ON)

#================================
# Define executable
#================================

if(BUILD_EXECUTABLE)
# Build vtkMNI utility lib (only needed by the executable)
   add_subdirectory(utils)

   add_executable( GPURigidRegistration main.cpp )
   target_link_libraries( GPURigidRegistration ${libName} vtkMNI )

else()
   # disable testing
   set(BUILD_TEST OFF)
   message( WARNING "Testing has been disabled." )
endif()

#================================
# Define tests
#================================

if(BUILD_TEST)
   if(NOT BUILD_EXECUTABLE)
      message( SEND_ERROR "Need to generate executable for testing. Please set BUILD_EXECUTABLE ON or turn disable BUILD_TEST.\n" )
   else()
      include(ExternalProject)
      enable_testing()
      add_subdirectory(tests)
   endif()
endif()