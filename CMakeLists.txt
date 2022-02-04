cmake_minimum_required(VERSION 3.16.3)
project(crgc)

include(CTest)
enable_testing()

#set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Ofast")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

#set(CMAKE_C_COMPILER "gcc")
#set(CMAKE_CXX_COMPILER "g++")



set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

#find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
#find_package(emp-ot REQUIRED)
find_package(emp-ot REQUIRED PATHS /usr/local/cmake/)
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

# set(SOURCES "src/generator-main.cp"; "src/evaluator-main.cpp")
set(TARGETS "generator"; "evaluator")
# add_executable(${PROJECT_NAME} src/generator-main.cpp)

#Testing macro


add_subdirectory(src/circuitProcessor)
add_subdirectory(src/programs)

#set_source_files_properties(circuitEvaluator.cpp PROPERTIES COMPILE_FLAGS -O3)
#add_compile_options(-O3)

foreach(TARGET IN LISTS TARGETS)
    add_executable(${TARGET} src/${TARGET}-main.cpp)
    target_include_directories(${TARGET} PUBLIC src)
    target_link_directories(${TARGET} PUBLIC src)
    target_include_directories(${TARGET} PUBLIC src/circuitProcessor)
    target_link_directories(${TARGET} PUBLIC src/circuitProcessor)
    target_link_libraries(${TARGET} helperFunctions circuitReader circuitEvaluator circuitFlipper circuitObfuscator circuitDetails circuitIntegrityBreaker circuitWriter circuitTransformer circuitCompressor leakagePredictor Threads::Threads)
    
    target_include_directories(${TARGET} PUBLIC src/programs)
    target_link_directories(${TARGET} PUBLIC src/programs)
    target_link_libraries(${TARGET} circuitLinker examplePrograms)
    
    target_link_libraries(${TARGET} ${EMP-OT_LIBRARIES})
    target_include_directories(${TARGET} PUBLIC TurboPFor-Integer-Compression)
    target_link_directories(${TARGET} PUBLIC TurboPFor-Integer-Compression)
    target_link_libraries(${TARGET} cir ic)

    #add_compile_options(-Ofast)
    #target_link_libraries(${TARGET} circuitEvaluator)

    
endforeach(SOURCE IN LISTS TARGETS)

#add_executable(generator src/circuitProcessor/helperFunctions.cpp src/circuitProcessor/circuitReader.cpp src/circuitProcessor/circuitEvaluator.cpp src/generator-main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
