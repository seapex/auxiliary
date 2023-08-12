# define the executable target
set (EXE_TARGET "sys_mngr")
#message("EXE_TARGET: ${EXE_TARGET}")

set(COMMON_PATH "../../../emctrl/common")
set(COMMU_PATH "${COMMON_PATH}/commu")
include(${COMMU_PATH}/buffer/guic/CMakeLists.txt)
include(${COMMU_PATH}/CMakeLists.txt)
add_sources(${COMMU_PATH}/socket_client.cpp)
add_sources(${COMMU_PATH}/protocol/phy_prtcl_pqb.cpp)
add_sources(${COMMU_PATH}/protocol/app_prtcl_gui.cpp ${COMMU_PATH}/protocol/app_prtcl_guic.cpp)
include(${COMMON_PATH}/data/CMakeLists.txt)
add_sources(${COMMON_PATH}/data/messageq_client.cpp ${COMMON_PATH}/data/messageq_guic.cpp)
include(${COMMON_PATH}/security/CMakeLists.txt)
add_sources(${COMMON_PATH}/security/sm4.cpp)
include(${COMMON_PATH}/config/CMakeLists.txt)
include(${COMMON_PATH}/misc/CMakeLists.txt)
include(${COMMON_PATH}/thread/CMakeLists.txt)
include(${COMMON_PATH}/time/CMakeLists.txt)

add_sources(main.cpp)

set(GUI_PATH "../../../emctrl/gui")
include_directories (${GUI_PATH}/src/data ${GUI_PATH}/src/config)
aux_source_directory(${GUI_PATH}/src/data DATA_SRC)
aux_source_directory(${GUI_PATH}/src/config CONFIG_SRC)

set (SRC_LIST ${SRC_LIST} ${DATA_SRC} ${CONFIG_SRC})
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
# set_property(TARGET ${EXE_TARGET} PROPERTY LINK_FLAGS -static-libgcc)
set_property(TARGET ${EXE_TARGET} PROPERTY LINK_FLAGS -shared-libgcc)

# specifies the directory into which runtime target files should be built
#set_property( TARGET ${EXE_TARGET} PROPERTY RUNTIME_OUTPUT_DIRECTORY
#              ${PROJECT_BINARY_DIR}/bin )

# add the install targets
install (TARGETS ${EXE_TARGET} DESTINATION /home/works/rootfs/mini/usr/local/bin)
#install (FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h" DESTINATION include)
