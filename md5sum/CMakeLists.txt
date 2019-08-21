
cmake_minimum_required (VERSION 2.8)
project (HaveATest)

# The version number.
set (_VERSION_MAJOR 1)
set (_VERSION_MINOR 0)
set (_VERSION_PATCH 1)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories ("${PROJECT_BINARY_DIR}")

# add the src/ to the search path for src files
add_subdirectory (src bin)

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${usermanage_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${usermanage_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${usermanage_VERSION_PATCH}")
include (CPack)

