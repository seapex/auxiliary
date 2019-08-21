# define the executable target
set (EXE_TARGET "sys_mngr")
#message("EXE_TARGET: ${EXE_TARGET}")

add_sources(main.cpp)
include(../../../emctrl/common/commu/CMakeLists_c.txt)
include(../../../emctrl/common/data/CMakeLists_c.txt)
include(../../../emctrl/common/time/CMakeLists.txt)
include(../../../emctrl/common/thread/CMakeLists.txt)
include(../../../emctrl/common/config/CMakeLists.txt)
include(../../../emctrl/common/misc/CMakeLists.txt)
include(../../../emctrl/common/security/CMakeLists_c.txt)

include_directories (../../../emctrl/gui/src/data ../../../emctrl/gui/src/config)

aux_source_directory(../../../emctrl/gui/src/data DATA_SRC)
aux_source_directory(../../../emctrl/gui/src/config CONFIG_SRC)

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
