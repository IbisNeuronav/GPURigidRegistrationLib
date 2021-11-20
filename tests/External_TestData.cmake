#--------------------------------------------------------------------------
# TestData
IF(NOT DEFINED(BUILD_TEST_GIT_REPOSITORY))
  SET(BUILD_TEST_GIT_REPOSITORY "${GIT_PROTOCOL}://github.com/IbisNeuronav/RegistrationImageDataExamples.git" CACHE STRING "Set TestData desired git url.")
ENDIF()
IF(NOT DEFINED(BUILD_TEST_GIT_REVISION))
  SET(BUILD_TEST_GIT_REVISION "main" CACHE STRING "Set TestData desired git hash (main means latest).")
ENDIF()

IF(NOT DEFINED(TestData))
  SET (TestData ${CMAKE_BINARY_DIR}/testData CACHE INTERNAL "Path to store testData contents.")
ENDIF()

ExternalProject_Add(TestData
  TIMEOUT 1000
  PREFIX "${TestData}"
  SOURCE_DIR "${TestData}/data"
  #--Download step--------------
  GIT_REPOSITORY ${BUILD_TEST_GIT_REPOSITORY}
  GIT_TAG ${BUILD_TEST_GIT_REVISION}
  #--Configure step-------------
  CONFIGURE_COMMAND ""
  #--Build step-----------------
  BUILD_COMMAND ""
  #--Install step-----------------
  INSTALL_COMMAND ""
  )
  
set(TestDataDir ${TestData}/data)