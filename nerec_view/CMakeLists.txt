cmake_minimum_required (VERSION 2.8)
project (NERecView)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_definitions(-DOSbeWindows)
endif()

# The version number.
set (_VERSION_MAJOR 1)
set (_VERSION_MINOR 0)
set (_VERSION_PATCH 0)

macro (add_sources)
    file (RELATIVE_PATH _relPath "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_LIST_DIR}")
    message("ARGN=${ARGN}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND SRC_LIST "${_relPath}/${_src}")
        else()
            list (APPEND SRC_LIST "${_src}")
        endif()
    endforeach()
    if (_relPath) # propagate SRC_LIST to parent directory
        set (SRC_LIST ${SRC_LIST} PARENT_SCOPE)
    endif()
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

# Copy external dependency files.
find_path(EMCTRL_PATH ".gitignore" "../../emctrl")
message("EMCTRL_PATH: ${EMCTRL_PATH}")
message("PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
configure_file (${EMCTRL_PATH}/common/data/ne_msg_dif.h ${PROJECT_SOURCE_DIR}/src COPYONLY)
configure_file (${EMCTRL_PATH}/common/config/param_phd.h ${PROJECT_SOURCE_DIR}/src/misc COPYONLY)
configure_file (${EMCTRL_PATH}/common/time/time_cst.cpp ${PROJECT_SOURCE_DIR}/src/misc COPYONLY)
configure_file (${EMCTRL_PATH}/common/time/time_cst.h ${PROJECT_SOURCE_DIR}/src/misc COPYONLY)
#file(COPY ${EMCTRL_PATH}/common/data/ne_msg_dif.h DESTINATION ${PROJECT_SOURCE_DIR}/src/misc)

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${usermanage_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${usermanage_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${usermanage_VERSION_PATCH}")
include (CPack)

