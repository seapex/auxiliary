file (RELATIVE_PATH _pwd "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_LIST_DIR}")
include_directories(${_pwd})
aux_source_directory(${_pwd} THREAD_SRC)

set ( SRC_LIST ${SRC_LIST} ${THREAD_SRC})

#unset(_pwd)
