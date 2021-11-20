#set( TestData ${CMAKE_BINARY_DIR}/testData CACHE INTERNAL "Path to store testData contents." )
#set( TestDataDir ${TestData}/data CACHE INTERNAL "Path to store testData contents." )

set(BUILD_TEST_GIT_REPOSITORY "${GIT_PROTOCOL}://github.com/IbisNeuronav/RegistrationImageDataExamples.git" CACHE STRING "Set TestData desired git url.")
mark_as_advanced(BUILD_TEST_GIT_REPOSITORY)
set(BUILD_TEST_GIT_REVISION "main" CACHE STRING "Set TestData desired git hash (main means latest).")
mark_as_advanced(BUILD_TEST_GIT_REVISION)

include(${CMAKE_CURRENT_SOURCE_DIR}/External_TestData.cmake)

add_executable(ImageRegistrationTest ImageRegistrationTest.cxx)
add_executable(ImageRegistrationWithInitialTransformTest ImageRegistrationWithInitialTransformTest.cxx)
add_executable(ImageRegistrationWithMaskTest ImageRegistrationWithMaskTest.cxx)

add_test(NAME ImageRegistrationTest COMMAND ImageRegistrationTest ${TestDataDir})
add_test(NAME ImageRegistrationWithInitialTransformTest COMMAND ImageRegistrationWithInitialTransformTest ${TestDataDir})
add_test(NAME ImageRegistrationWithMaskTest COMMAND ImageRegistrationWithMaskTest ${TestDataDir})

target_link_libraries( ImageRegistrationTest ${VTK_LIBRARIES} ${libName} )
target_link_libraries( ImageRegistrationWithInitialTransformTest ${VTK_LIBRARIES} ${libName} ${vtkMNI})
target_link_libraries( ImageRegistrationWithMaskTest ${VTK_LIBRARIES} ${libName} ${vtkMNI})