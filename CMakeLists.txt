cmake_minimum_required(VERSION 3.10)
project(oblig3)

# @doc - Windows Cmake makefile issue -  https://stackoverflow.com/questions/41333215/mingw-sh-exe-must-not-be-in-your-path -05.03.18
add_executable(oblig3 main.cpp)
set_target_properties(oblig3
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "bin"
    LIBRARY_OUTPUT_DIRECTORY "lib"
    BUILD_TYPE "Debug")

target_compile_options(oblig3
    PRIVATE "-Wall"
    PRIVATE "-Wextra")

target_compile_features(oblig3
    PRIVATE cxx_lambdas
    PRIVATE cxx_generic_lambdas
)