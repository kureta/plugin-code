####################################################################################################
# basic project config
cmake_minimum_required(VERSION 3.5)
set(project_name "Performer")
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_STANDARD 17)

####################################################################################################
# load modules
include(SuperColliderServerPlugin RESULT_VARIABLE server_plugin_found)
if(NOT server_plugin_found)
    message(FATAL_ERROR "Could not find server plugin functions module")
endif()

include(SuperColliderCompilerConfig RESULT_VARIABLE compiler_config_found)
if(NOT compiler_config_found)
    message(FATAL_ERROR "Could not find compiler config module")
endif()

# Windows - puts redistributable DLLs in install directory
include(InstallRequiredSystemLibraries)

# Environment variables
set(SC_PATH /home/devuser/lib/supercollider)
set(CMAKE_INSTALL_PREFIX /workspaces/plugin-code/packages)
####################################################################################################

sc_check_sc_path("${SC_PATH}")
message(STATUS "Found SuperCollider: ${SC_PATH}")
set(SC_PATH "${SC_PATH}" CACHE PATH
    "Path to SuperCollider source. Relative paths are treated as relative to this script" FORCE)

include("${SC_PATH}/SCVersion.txt")
message(STATUS "Building plugins for SuperCollider version: ${SC_VERSION}")

# set project here to avoid SCVersion.txt clobbering our version info
project(${project_name})
sc_do_initial_compiler_config() # do after setting project so compiler ID is available

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR NOT CMAKE_INSTALL_PREFIX)
    message(WARNING "No install prefix provided, defaulting to $BUILD_DIR/install")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install prefix" FORCE)
endif()

message(STATUS "Install directory set to: ${CMAKE_INSTALL_PREFIX}")

# Find LibTorch C++
set(CMAKE_MODULE_PATH "/home/devuser/lib/libtorch" ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH "/home/devuser/lib/libtorch" ${CMAKE_PREFIX_PATH})
set(TORCH_CUDA_ARCH_LIST "7.5;5.2")
find_package(Torch REQUIRED)

####################################################################################################
# options
option(SUPERNOVA "Build plugins for supernova" OFF)
option(SCSYNTH "Build plugins for scsynth" ON)
option(NATIVE "Optimize for native architecture" ON)
option(STRICT "Use strict warning flags" OFF)
option(NOVA_SIMD "Build plugins with nova-simd support." OFF)

####################################################################################################
# include libraries

if (NOVA_SIMD)
	add_definitions(-DNOVA_SIMD)
	include_directories(${SC_PATH}/external_libraries/nova-simd)
endif()

####################################################################################################
# Begin target Performer

set(Performer_cpp_files
    plugins/Performer/Performer.cpp
    plugins/Performer/Performer.hpp
)
set(Performer_sc_files
    plugins/Performer/Performer.sc
)
set(Performer_schelp_files
    plugins/Performer/Performer.schelp
)

sc_add_server_plugin(
    "Performer/Performer" # desination directory
    "Performer" # target name
    "${Performer_cpp_files}"
    "${Performer_sc_files}"
    "${Performer_schelp_files}"
)

# End target Performer
####################################################################################################

target_link_libraries(Performer_scsynth "${TORCH_LIBRARIES}")

####################################################################################################
# END PLUGIN TARGET DEFINITION
####################################################################################################

message(STATUS "Generating plugin targets done")
