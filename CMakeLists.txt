cmake_minimum_required (VERSION 3.1.0)
project (spre_test)

include_directories(include)
file(GLOB SOURCES "test/*.cpp")

add_executable(spre_test ${SOURCES})

set_property(TARGET spre_test PROPERTY CXX_STANDARD 14)
set_property(TARGET spre_test PROPERTY CXX_STANDARD_REQUIRED ON)
