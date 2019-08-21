# define the executable target
set (EXE_TARGET "md5sum")
#message("EXE_TARGET: ${EXE_TARGET}")

include_directories (hash)
aux_source_directory(hash LIB_SRC)
set (SRC_LIST ${SRC_LIST} ${LIB_SRC} md5sum.cpp)
#set (SRC_LIST ${SRC_LIST} main.c)
#set (EXTRA_LIBS ${EXTRA_LIBS} pthread dl)
set (EXTRA_LIBS ${EXTRA_LIBS} m)
# add the executable
add_executable (${EXE_TARGET} ${SRC_LIST})
target_link_libraries (${EXE_TARGET} ${EXTRA_LIBS})

# Set the GCC Additional flags to use when compiling this target's sources
# -O0 -O1 -O2 -O3 ...
set_property(TARGET ${EXE_TARGET} PROPERTY COMPILE_FLAGS -O2)
# set_property(TARGET ${EXE_TARGET} PROPERTY COMPILE_FLAGS -DDEBUG_MOD)

# Set the GCC Additional flags to use when linking this target
# -shared-libgcc -static-libstdc++ -static-libgcc ...
# set_property(TARGET ${EXE_TARGET} PROPERTY LINK_FLAGS -static-libgcc)
set_property(TARGET ${EXE_TARGET} PROPERTY LINK_FLAGS -shared-libgcc)

# specifies the directory into which runtime target files should be built
#set_property( TARGET ${EXE_TARGET} PROPERTY RUNTIME_OUTPUT_DIRECTORY
#              ${PROJECT_BINARY_DIR}/bin )

# add the install targets
install (TARGETS ${EXE_TARGET} DESTINATION bin)
#install (FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h" DESTINATION include)
