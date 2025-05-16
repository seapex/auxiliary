
cmake_minimum_required (VERSION 2.8)
project (SysMngr)
add_definitions(-DDPQNetGUI)

# The version number.
set (_VERSION_MAJOR 1)
set (_VERSION_MINOR 1)
set (_VERSION_PATCH 0)

macro (add_sources)
    file (RELATIVE_PATH _relPath "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_LIST_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND SRC_LIST "${_relPath}/${_src}")
        else()
            list (APPEND SRC_LIST "${_src}")
        endif()
    endforeach()
endmacro()

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

