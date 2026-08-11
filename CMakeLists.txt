cmake_minimum_required(VERSION 3.10)
project(SalonSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(SalonSystem
    main.cpp
)

if(MSVC)
    target_compile_options(SalonSystem PRIVATE /EHsc)
endif()
