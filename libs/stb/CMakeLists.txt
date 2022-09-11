set(LIBRARY_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/")

add_library(stb INTERFACE)

target_include_directories(stb INTERFACE ${LIBRARY_INCLUDE_DIR})
