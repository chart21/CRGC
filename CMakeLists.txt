cmake_minimum_required(VERSION 3.16.3)
project(generator)

include(CTest)
enable_testing()


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} src/generator-main.cpp)

add_subdirectory(src/circuitProcessor)

#target_include_directories(${PROJECT_NAME} PUBLIC src/circuits)
#target_link_directories(${PROJECT_NAME} PUBLIC src/circuits)


target_include_directories(${PROJECT_NAME} PUBLIC src/circuitProcessor)
target_link_directories(${PROJECT_NAME} PUBLIC src/circuitProcessor)

target_link_libraries(${PROJECT_NAME} helperFunctions circuitReader circuitEvaluator circuitFlipper circuitObfuscator circuitDetails circuitIntegrityBreaker circuitWriter leakagePredictor Threads::Threads)

#add_executable(generator src/circuitProcessor/helperFunctions.cpp src/circuitProcessor/circuitReader.cpp src/circuitProcessor/circuitEvaluator.cpp src/generator-main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
