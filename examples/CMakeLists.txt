add_compile_definitions(-DTINYPLY_AS_LIBRARY)
add_executable(cube cube.cpp)


set(examplebindir ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_custom_command(
    TARGET cube
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:cube>  ${examplebindir}
)
