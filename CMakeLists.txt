cmake_minimum_required(VERSION 3.0.0)
project(generator)

include(CTest)
enable_testing()

add_executable(${PROJECT_NAME} src/generator-main.cpp)

add_subdirectory(src/circuitProcessor)

target_include_directories(${PROJECT_NAME} PUBLIC src/circuitProcessor)
target_link_directories(${PROJECT_NAME} PUBLIC src/circuitProcessor)

target_link_libraries(${PROJECT_NAME} helperFunctions circuitReader circuitEvaluator)

#add_executable(generator src/circuitProcessor/helperFunctions.cpp src/circuitProcessor/circuitReader.cpp src/circuitProcessor/circuitEvaluator.cpp src/generator-main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
