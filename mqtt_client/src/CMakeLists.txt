# define the executable target
set (EXE_TARGET "mqtt_client")
#message("EXE_TARGET: ${EXE_TARGET}")
message("_OS_TYPE: ${_OS_TYPE}")

add_sources(main.cpp thrd_socket.cpp thrd_timer.cpp mqtt_client.cpp parse_optn_mqttc.cpp)
set(COMMON_PATH "/home/works/emctrl/common")

include(${COMMON_PATH}/commu/CMakeLists.txt)
include(CMakeLists4commu_mqttc.txt)

include(${COMMON_PATH}/data/CMakeLists.txt)
add_sources(${COMMON_PATH}/data/messageq_client.cpp ${COMMON_PATH}/data/messageq_mqttc.cpp)

include(${COMMON_PATH}/parse_optn/CMakeLists.txt)
include(${COMMON_PATH}/misc/CMakeLists.txt)
include(${COMMON_PATH}/thread/CMakeLists.txt)
include(${COMMON_PATH}/time/CMakeLists.txt)

include_directories (config data)
aux_source_directory(config CONFIG_SRC)
aux_source_directory(data DATA_SRC)

set ( SRC_LIST ${SRC_LIST} ${CONFIG_SRC} ${DATA_SRC})
message("SRC_LIST=${SRC_LIST}")

#set (EXTRA_LIBS ${EXTRA_LIBS} pthread dl)
set (EXTRA_LIBS ${EXTRA_LIBS} m pthread z)
# add the executable
add_executable (${EXE_TARGET} ${SRC_LIST})
target_link_libraries (${EXE_TARGET} ${EXTRA_LIBS})

# Set the GCC Additional flags to use when compiling this target's sources
# -O0 -O1 -O2 -O3 ...
set_property(TARGET ${EXE_TARGET} PROPERTY COMPILE_FLAGS -O2)

# Set the GCC Additional flags to use when linking this target
# -shared-libgcc -static-libstdc++ -static-libgcc ...
set_property(TARGET ${EXE_TARGET} PROPERTY LINK_FLAGS -static-libgcc)

# specifies the directory into which runtime target files should be built
#set_property( TARGET ${EXE_TARGET} PROPERTY RUNTIME_OUTPUT_DIRECTORY
#              ${PROJECT_BINARY_DIR}/bin )

# add the install targets
install (TARGETS ${EXE_TARGET} DESTINATION /home/works/install/upfile)
#install (FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h" DESTINATION include)
