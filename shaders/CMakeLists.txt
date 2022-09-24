# Compilation of shaders
find_package(Vulkan COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

file (GLOB SHADERS
   ${SHADERS_SOURCE_DIR}/*.vert
   ${SHADERS_SOURCE_DIR}/*.frag
)

foreach(source IN LISTS SHADERS)
   get_filename_component(FILENAME ${source} NAME)

   get_filename_component(NAME_WITHOUT_EXT ${source} NAME_WE)

   get_filename_component(TMP_EXTENSION ${source} EXT)
   string(REPLACE "." "" EXTENSION ${TMP_EXTENSION})

   set(OUTPUT_FILENAME "${EXTENSION}-${NAME_WITHOUT_EXT}")

   add_custom_command(
      COMMAND ${glslc_executable} ${source} -o ${OUTPUT_FILENAME}.spv
      OUTPUT ${SHADERS_BINARY_DIR}/${OUTPUT_FILENAME}.spv
      COMMENT "Compiling ${FILENAME}"
   )

   list(APPEND SPV_SHADERS ${SHADERS_BINARY_DIR}/${OUTPUT_FILENAME}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
