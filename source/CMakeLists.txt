# Build as library

set(libname tinyply)

option(SHARED_LIB "   - shared library" OFF)
if(SHARED_LIB)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    add_library(${libname} SHARED tinyply.cpp)
else()
    add_library(${libname} STATIC tinyply.cpp)
endif()
set(bindir ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

add_custom_command(
    TARGET ${libname}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${libname}>  ${bindir}
)
