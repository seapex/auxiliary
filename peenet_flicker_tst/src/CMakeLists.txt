# define the executable target
set (EXE_TARGET "flicker_tst")
#message("EXE_TARGET: ${EXE_TARGET}")

add_sources(main.cpp flicker_filter.c flicker_statis.c)
aux_source_directory(parse_optn PARSE_OPT_SRC)

set (ExtPath "../../../emctrl/common")
add_sources(${ExtPath}/time/time_cst.cpp)
#add_sources(${ExtPath}/misc/math_ext.cpp ${ExtPath}/misc/generic.cpp)

include_directories (parse_optn ${ExtPath}/misc ${ExtPath}/time)

set (SRC_LIST ${SRC_LIST} ${PARSE_OPT_SRC})
#set (EXTRA_LIBS ${EXTRA_LIBS} pthread dl)
#set (EXTRA_LIBS ${EXTRA_LIBS} m z)

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
install (TARGETS ${EXE_TARGET} DESTINATION bin)
#install (FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h" DESTINATION include)
