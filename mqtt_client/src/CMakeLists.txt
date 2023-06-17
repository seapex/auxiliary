# define the executable target
set (EXE_TARGET "mqtt_client")
#message("EXE_TARGET: ${EXE_TARGET}")
message("_OS_TYPE: ${_OS_TYPE}")

set(COMMON_PATH "/home/works/emctrl/common")
set(COMMU_PATH "${COMMON_PATH}/commu")
include(${COMMU_PATH}/buffer/mqttc/CMakeLists.txt)
include(${COMMU_PATH}/CMakeLists.txt)
add_sources(${COMMU_PATH}/socket_client.cpp)
add_sources(${COMMU_PATH}/protocol/phy_prtcl_mqttc.cpp)
add_sources(${COMMU_PATH}/protocol/app_prtcl_mqttc.cpp)
include(${COMMON_PATH}/config/CMakeLists.txt)
include(${COMMON_PATH}/data/CMakeLists.txt)
add_sources(${COMMON_PATH}/data/messageq_client.cpp ${COMMON_PATH}/data/messageq_mqttc.cpp)
include(${COMMON_PATH}/parse_optn/CMakeLists.txt)
include(${COMMON_PATH}/misc/CMakeLists.txt)
include(${COMMON_PATH}/mqtt_client/CMakeLists.txt)
include(${COMMON_PATH}/thread/CMakeLists.txt)
include(${COMMON_PATH}/time/CMakeLists.txt)

add_sources(main.cpp thrd_socket.cpp thrd_timer.cpp parse_optn_mqttc.cpp)
include_directories (test)
aux_source_directory(test TEST_SRC)

set ( SRC_LIST ${SRC_LIST} ${TEST_SRC})

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
