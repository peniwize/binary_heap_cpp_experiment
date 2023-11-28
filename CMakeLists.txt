cmake_minimum_required(VERSION 3.12.4)
set(project_name heap_experiment_1)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU"
    OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -fanalyzer
        -Wall
        -Wconversion
        -Werror
        -Wextra
        -Wpedantic
        -Wshadow
    )
endif()
project(${project_name})
add_executable(
    ${project_name}
    main.cpp
)
target_include_directories(
    ${project_name}
    PUBLIC
        ./doctest
)