if(LIB)
    add_compile_definitions(-DTINYPLY_LINK_AS_LIBRARY)

    add_executable(cube cube.cpp)
    set(bindir ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
    target_link_libraries(cube PRIVATE ${bindir}/libtinyply.a)
else()
    add_executable(cube cube.cpp)
endif()

set(examplebindir ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_custom_command(
    TARGET cube
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:cube>  ${examplebindir}
)
