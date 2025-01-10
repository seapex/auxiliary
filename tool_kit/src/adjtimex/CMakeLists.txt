# define the executable target
set (EXE_TARGET "adjtimex")
#message("PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")
add_sources(adjtimex.c)

#set (EXTRA_LIBS ${EXTRA_LIBS} rt)
# add the executable
add_executable (${EXE_TARGET} ${SRC_LIST})
target_link_libraries (${EXE_TARGET} ${EXTRA_LIBS})

# Set the GCC Additional flags to use when compiling this target's sources
# -O0 -O1 -O2 -O3 ...
set_property(TARGET ${EXE_TARGET} PROPERTY COMPILE_FLAGS "-std=c99")
#set_property(TARGET ${EXE_TARGET} PROPERTY COMPILE_FLAGS "-D_POSIX_C_SOURCE=199309 -D_BSD_SOURCE")
set_property(TARGET ${EXE_TARGET} PROPERTY COMPILE_FLAGS "-O2 -W -Wall")

# Set the GCC Additional flags to use when linking this target
# -shared-libgcc -static-libstdc++ -static-libgcc ...
set_property(TARGET ${EXE_TARGET} PROPERTY LINK_FLAGS -static-libgcc)

# specifies the directory into which runtime target files should be built
#set_property( TARGET ${EXE_TARGET} PROPERTY RUNTIME_OUTPUT_DIRECTORY
#              ${PROJECT_BINARY_DIR}/bin )

add_custom_command(TARGET ${EXE_TARGET} POST_BUILD 
  COMMAND cp ${PROJECT_BINARY_DIR}/adjtimex/${EXE_TARGET} ${CMAKE_HOME_DIRECTORY}/release -a
  COMMENT "Copy ${EXE_TARGET} to ${CMAKE_HOME_DIRECTORY}/release/" )

# add the install targets
install (TARGETS ${EXE_TARGET} DESTINATION bin)
#install (FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h" DESTINATION include)
