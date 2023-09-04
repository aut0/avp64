##############################################################################
#                                                                            #
# Copyright 2020 Lukas Jünger                                                #
#                                                                            #
# This software is licensed under the MIT license.                           #
# A copy of the license can be found in the LICENSE file at the root         #
# of the source tree.                                                        #
#                                                                            #
#############################################################################

cmake_minimum_required(VERSION 3.6)
project(avp64)
option(AVP64_BUILD_TESTS "Build unit tests" OFF)

include(ExternalProject)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(AVP64_LINTER "" CACHE STRING "Code linter to use")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_EXE_LINKER_FLAGS -rdynamic)

set(OCX_QEMU_ARM_HOME $ENV{OCX_QEMU_HOME})
if(NOT EXISTS ${OCX_QEMU_ARM_HOME})
    set(OCX_QEMU_ARM_HOME ${CMAKE_CURRENT_SOURCE_DIR}/deps/ocx-qemu-arm)
endif()

set(VCML_HOME $ENV{VCML_HOME})
if(NOT EXISTS ${VCML_HOME})
    set(VCML_HOME ${CMAKE_CURRENT_SOURCE_DIR}/deps/vcml)
endif()

message(STATUS "Found OCX-QEMU-ARM at " ${OCX_QEMU_ARM_HOME})
message(STATUS "Found VCML at " ${VCML_HOME})

add_subdirectory(${OCX_QEMU_ARM_HOME} ocx-qemu-arm)
add_subdirectory(${VCML_HOME} vcml)

set(src "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(inc "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/ocx-qemu-arm/ocx/include")

set(sources
    ${src}/arm64_cpu.cpp
    ${src}/system.cpp
    ${src}/main.cpp)

add_executable(avp64 ${sources})

set_target_properties(avp64 PROPERTIES CXX_CLANG_TIDY "${AVP64_LINTER}")

target_include_directories(avp64 PRIVATE ${inc})

target_link_libraries(avp64 vcml)
target_link_libraries(avp64 ${CMAKE_DL_LIBS})
target_link_libraries(avp64 -pthread -lelf)

install(TARGETS avp64 DESTINATION bin)
install(DIRECTORY config/ DESTINATION config)

if(AVP64_BUILD_TESTS)
    message(STATUS "Building AVP64 tests")
    enable_testing()
    add_subdirectory(tests)
endif()
