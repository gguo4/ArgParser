cmake_minimum_required (VERSION 3.9)
MESSAGE(STATUS "CMAKE_ROOT: " ${CMAKE_ROOT})

project(ArgParser)

set(ARGPARSER_MAJOR_VERSION "1")
set(ARGPARSER_MINOR_VERSION "0")
set(CMAKE_VERBOSE_MAKEFILE on)


# cmake ../ -DCMAKE_CXX_COMPILER=clang++
# cmake ../ -DCMAKE_CXX_COMPILER=g++
message(STATUS "COMPILER: " ${CMAKE_CXX_COMPILER_ID} " VERSION: " ${CMAKE_CXX_COMPILER_VERSION})
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.2")
    message(FATAL_ERROR "g++ version must be at least 7.2")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5")
    message(FATAL_ERROR "clang++ version must be at least 5")
  endif()
else()
  message(FATAL_ERROR "unsupported compiler (must be either g++ or clang++)")
endif()


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/doctest)

# make the example binary in exampl/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/example)
add_executable(example example/example.cpp)

enable_testing()

message(STATUS "Building unit tests ...")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/unittest)
add_executable(argparser unittest/test.cpp)

add_test(option ${PROJECT_SOURCE_DIR}/unittest/argparser -tc=ArgParser.Option)
add_test(flag ${PROJECT_SOURCE_DIR}/unittest/argparser -tc=ArgParser.Flag)
add_test(multiopt ${PROJECT_SOURCE_DIR}/unittest/argparser -tc=ArgParser.MultiOpt)
add_test(failure ${PROJECT_SOURCE_DIR}/unittest/argparser -tc=ArgParser.Failures)
add_test(partialopt ${PROJECT_SOURCE_DIR}/unittest/argparser -tc=ArgParser.PartialOptions)


