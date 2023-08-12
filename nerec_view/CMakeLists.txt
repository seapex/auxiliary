cmake_minimum_required (VERSION 2.8)
project (NERecView)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_definitions(-DOSbeWindows)
endif()

# The version number.
set (_VERSION_MAJOR 1)
set (_VERSION_MINOR 0)
set (_VERSION_PATCH 2)

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

# Update external dependency files.
find_path(EMCTRL_PATH ".gitignore" "../../emctrl")
set(DEPSRC_PATH "${EMCTRL_PATH}/common")
set(DEPDES_PATH "${PROJECT_SOURCE_DIR}/src")

add_custom_command(
   OUTPUT ${DEPDES_PATH}/ne_msg_dif.h
   COMMAND ${CMAKE_COMMAND} -E copy ${DEPSRC_PATH}/data/ne_msg_dif.h ${DEPDES_PATH}/
   COMMENT "Updating ne_msg_dif.h"
   DEPENDS ${DEPSRC_PATH}/data/ne_msg_dif.h
)
add_custom_command(
   OUTPUT ${DEPDES_PATH}/misc/param_phd.h
   COMMAND ${CMAKE_COMMAND} -E copy ${DEPSRC_PATH}/config/param_phd.h ${DEPDES_PATH}/misc/
   COMMENT "Updating param_phd.h"
   DEPENDS ${DEPSRC_PATH}/config/param_phd.h
)
add_custom_command(
   OUTPUT "${DEPDES_PATH}/misc/time_cst.cpp" "${DEPDES_PATH}/misc/time_cst.h"
   COMMAND ${CMAKE_COMMAND} -E copy ${DEPSRC_PATH}/time/time_cst.cpp ${DEPDES_PATH}/misc/
   COMMAND ${CMAKE_COMMAND} -E copy ${DEPSRC_PATH}/time/time_cst.h ${DEPDES_PATH}/misc/
   COMMENT "Updating time_cst.*"
   DEPENDS ${DEPSRC_PATH}/time/time_cst.cpp ${DEPSRC_PATH}/time/time_cst.h
)

add_custom_target(
   update_depfile ALL
   DEPENDS "${DEPDES_PATH}/ne_msg_dif.h" "${DEPDES_PATH}/misc/param_phd.h" 
           "${DEPDES_PATH}/misc/time_cst.cpp" "${DEPDES_PATH}/misc/time_cst.h"
)

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${usermanage_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${usermanage_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${usermanage_VERSION_PATCH}")
include (CPack)

