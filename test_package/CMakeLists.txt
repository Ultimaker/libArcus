project(PackageTest)
cmake_minimum_required(VERSION 3.20)

include(../cmake/StandardProjectSettings.cmake)

find_package(Arcus REQUIRED)

add_executable(test test.cpp)
set_project_standards(test)
set_project_warnings(test)
use_threads(test)

target_link_libraries(test
        PUBLIC
        Arcus
        )