# define the executable target
set (EXE_TARGET "nerec_view")
#message("EXE_TARGET: ${EXE_TARGET}")

add_sources(main.cpp)
include(misc/CMakeLists.txt)
#include(../../../emctrl/common/time/CMakeLists.txt)

set (SRC_LIST ${SRC_LIST} main.cpp)
#set (EXTRA_LIBS ${EXTRA_LIBS} m pthread dl)
set (EXTRA_LIBS ${EXTRA_LIBS} z pthread)
# add the executable
add_executable (${EXE_TARGET} ${SRC_LIST})
target_link_libraries (${EXE_TARGET} ${EXTRA_LIBS})

# Set the GCC Additional flags to use when compiling this target's sources
# -O0 -O1 -O2 -O3 ...
set_property(TARGET ${EXE_TARGET} PROPERTY COMPILE_FLAGS -O2)

# Set the GCC Additional flags to use when linking this target
# -shared-libgcc -static-libstdc++ -static-libgcc ...
message(${CMAKE_SYSTEM_NAME})
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_property(TARGET ${EXE_TARGET} PROPERTY LINK_FLAGS -static)
else()
    # set_property(TARGET ${EXE_TARGET} PROPERTY LINK_FLAGS -static-libgcc)
endif()

# specifies the directory into which runtime target files should be built
#set_property( TARGET ${EXE_TARGET} PROPERTY RUNTIME_OUTPUT_DIRECTORY
#              ${PROJECT_BINARY_DIR}/bin )

# add the install targets
install (TARGETS ${EXE_TARGET} DESTINATION bin)
#install (FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h" DESTINATION include)
