cmake_minimum_required(VERSION 3.22.1)
project(diag_native)

add_library(diag_native SHARED
    src/main/cpp/native_diag.cpp)

find_library(log-lib log)

target_compile_features(diag_native PRIVATE cxx_std_17)

target_compile_options(diag_native PRIVATE
    -Wall
    -Wextra
    -Werror)

target_link_libraries(diag_native
    ${log-lib})
