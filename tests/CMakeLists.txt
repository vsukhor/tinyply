# Self-contained tests: all PLY data is generated in memory or in the system
# temp directory.

add_executable(tests tests.cpp tinyply_impl.cpp)
add_test(NAME tinyply COMMAND tests)
