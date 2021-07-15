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

#find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
find_package(emp-ot REQUIRED)
#find_package(emp-sh2pc REQUIRED)
INCLUDE_DIRECTORIES(${EMP-OT_INCLUDE_DIRS}) #recently added

# macro (add_test_executable_with_lib _name libs)
# 	add_executable(test_${_name} "${_name}.cpp")
# 	target_link_libraries(test_${_name} ${EMP-OT_LIBRARIES})
# endmacro()

# macro (add_test_case _name)
# 	add_test_executable_with_lib(${_name} "")
#   	add_test(NAME ${_name} COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_${_name}" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/")
# endmacro()

# macro (add_test_case_with_run _name)
# 	add_test_executable_with_lib(${_name} "")
# 	add_test(NAME ${_name} COMMAND "./run" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_${_name}" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/")

# endmacro()

# # Test cases
# add_test_case_with_run(mult3)


add_executable(${PROJECT_NAME} src/generator-main.cpp)
#Testing macro


add_subdirectory(src/circuitProcessor)

#target_include_directories(${PROJECT_NAME} PUBLIC src/circuits)
#target_link_directories(${PROJECT_NAME} PUBLIC src/circuits)


target_include_directories(${PROJECT_NAME} PUBLIC src/circuitProcessor)
target_link_directories(${PROJECT_NAME} PUBLIC src/circuitProcessor)

target_link_libraries(${PROJECT_NAME} helperFunctions circuitReader circuitEvaluator circuitFlipper circuitObfuscator circuitDetails circuitIntegrityBreaker circuitWriter circuitTransformer leakagePredictor Threads::Threads)


add_subdirectory(src/programs)
target_include_directories(${PROJECT_NAME} PUBLIC src/programs)
target_link_directories(${PROJECT_NAME} PUBLIC src/programs)

target_link_libraries(${PROJECT_NAME} mult3 paperoptm circuitLinker)

target_link_libraries(${PROJECT_NAME} ${EMP-OT_LIBRARIES})
#add_executable(generator src/circuitProcessor/helperFunctions.cpp src/circuitProcessor/circuitReader.cpp src/circuitProcessor/circuitEvaluator.cpp src/generator-main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
