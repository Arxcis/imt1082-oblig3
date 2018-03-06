cmake_minimum_required(VERSION 3.10)
project(oblig3)

# @doc - Windows Cmake makefile issue -  https://stackoverflow.com/questions/41333215/mingw-sh-exe-must-not-be-in-your-path -05.03.18
add_executable(oblig3 main.cpp)

set(CMAKE_BUILD_TYPE "Debug")

set_target_properties(oblig3
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "bin"
    LIBRARY_OUTPUT_DIRECTORY "lib")

target_compile_options(oblig3
    PRIVATE "-Wall"
    PRIVATE "-Wno-unused-parameter"
    PRIVATE "-Wextra"
    PRIVATE "-Wno-self-assign")

target_compile_features(oblig3
    PRIVATE cxx_lambdas
    PRIVATE cxx_generic_lambdas
    PRIVATE cxx_variadic_templates
)