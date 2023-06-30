cmake_minimum_required(VERSION 3.2)

project(HomeWorkCPP5.5)

set(Postgre_ADDITIONAL_VERSIONS "15")

add_subdirectory("..\\libpqxx-7.7.5" libpqxx-build)

add_executable(HomeWorkCPP5.5 HomeWorkCPP5.5.cpp)

target_compile_features(HomeWorkCPP5.5 PRIVATE cxx_std_17)

target_link_libraries(HomeWorkCPP5.5 pqxx)