# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/christopher/Documents/C-RGCG

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/christopher/Documents/C-RGCG/build

# Include any dependencies generated for this target.
include src/circuitProcessor/CMakeFiles/helperFunctions.dir/depend.make

# Include the progress variables for this target.
include src/circuitProcessor/CMakeFiles/helperFunctions.dir/progress.make

# Include the compile flags for this target's objects.
include src/circuitProcessor/CMakeFiles/helperFunctions.dir/flags.make

src/circuitProcessor/CMakeFiles/helperFunctions.dir/helperFunctions.cpp.o: src/circuitProcessor/CMakeFiles/helperFunctions.dir/flags.make
src/circuitProcessor/CMakeFiles/helperFunctions.dir/helperFunctions.cpp.o: ../src/circuitProcessor/helperFunctions.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/christopher/Documents/C-RGCG/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/circuitProcessor/CMakeFiles/helperFunctions.dir/helperFunctions.cpp.o"
	cd /home/christopher/Documents/C-RGCG/build/src/circuitProcessor && /bin/x86_64-linux-gnu-g++-9  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/helperFunctions.dir/helperFunctions.cpp.o -c /home/christopher/Documents/C-RGCG/src/circuitProcessor/helperFunctions.cpp

src/circuitProcessor/CMakeFiles/helperFunctions.dir/helperFunctions.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/helperFunctions.dir/helperFunctions.cpp.i"
	cd /home/christopher/Documents/C-RGCG/build/src/circuitProcessor && /bin/x86_64-linux-gnu-g++-9 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/christopher/Documents/C-RGCG/src/circuitProcessor/helperFunctions.cpp > CMakeFiles/helperFunctions.dir/helperFunctions.cpp.i

src/circuitProcessor/CMakeFiles/helperFunctions.dir/helperFunctions.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/helperFunctions.dir/helperFunctions.cpp.s"
	cd /home/christopher/Documents/C-RGCG/build/src/circuitProcessor && /bin/x86_64-linux-gnu-g++-9 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/christopher/Documents/C-RGCG/src/circuitProcessor/helperFunctions.cpp -o CMakeFiles/helperFunctions.dir/helperFunctions.cpp.s

# Object files for target helperFunctions
helperFunctions_OBJECTS = \
"CMakeFiles/helperFunctions.dir/helperFunctions.cpp.o"

# External object files for target helperFunctions
helperFunctions_EXTERNAL_OBJECTS =

src/circuitProcessor/libhelperFunctions.a: src/circuitProcessor/CMakeFiles/helperFunctions.dir/helperFunctions.cpp.o
src/circuitProcessor/libhelperFunctions.a: src/circuitProcessor/CMakeFiles/helperFunctions.dir/build.make
src/circuitProcessor/libhelperFunctions.a: src/circuitProcessor/CMakeFiles/helperFunctions.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/christopher/Documents/C-RGCG/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libhelperFunctions.a"
	cd /home/christopher/Documents/C-RGCG/build/src/circuitProcessor && $(CMAKE_COMMAND) -P CMakeFiles/helperFunctions.dir/cmake_clean_target.cmake
	cd /home/christopher/Documents/C-RGCG/build/src/circuitProcessor && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/helperFunctions.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/circuitProcessor/CMakeFiles/helperFunctions.dir/build: src/circuitProcessor/libhelperFunctions.a

.PHONY : src/circuitProcessor/CMakeFiles/helperFunctions.dir/build

src/circuitProcessor/CMakeFiles/helperFunctions.dir/clean:
	cd /home/christopher/Documents/C-RGCG/build/src/circuitProcessor && $(CMAKE_COMMAND) -P CMakeFiles/helperFunctions.dir/cmake_clean.cmake
.PHONY : src/circuitProcessor/CMakeFiles/helperFunctions.dir/clean

src/circuitProcessor/CMakeFiles/helperFunctions.dir/depend:
	cd /home/christopher/Documents/C-RGCG/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/christopher/Documents/C-RGCG /home/christopher/Documents/C-RGCG/src/circuitProcessor /home/christopher/Documents/C-RGCG/build /home/christopher/Documents/C-RGCG/build/src/circuitProcessor /home/christopher/Documents/C-RGCG/build/src/circuitProcessor/CMakeFiles/helperFunctions.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/circuitProcessor/CMakeFiles/helperFunctions.dir/depend

