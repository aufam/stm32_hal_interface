file(READ version.txt PERIPH_VERSION)
message("PERIPH_VERSION : ${PERIPH_VERSION}")

# sources
file(GLOB_RECURSE PERIPH_SOURCES periph/*.*)
add_library(periph ${PERIPH_SOURCES})

# include dirs
target_include_directories(periph PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# defines
target_compile_definitions(periph PUBLIC -DPERIPH_VERSION="${PERIPH_VERSION}")