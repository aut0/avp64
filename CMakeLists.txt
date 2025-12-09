##############################################################################
#                                                                            #
# Copyright 2024 Lukas Jünger, Nils Bosbach                                  #
#                                                                            #
# This software is licensed under the MIT license.                           #
# A copy of the license can be found in the LICENSE file at the root         #
# of the source tree.                                                        #
#                                                                            #
##############################################################################

cmake_minimum_required(VERSION 3.12)
project(avp64 VERSION 2025.08.22 LANGUAGES C CXX)

option(AVP64_TESTS "Build unit tests" OFF)
option(AVP64_VP "Build the avp64 VP" ON)
set(AVP64_LINTER "" CACHE STRING "Code linter to use")

include(cmake/common.cmake)
find_github_repo(vcml "machineware-gmbh/vcml")

set(OCX_QEMU_ARM_BUILD_TESTS OFF CACHE BOOL "disable ocx tests")
find_github_repo(ocx-qemu-arm "nbosb/ocx-qemu-arm" "avp64")
set_target_properties(ocx-qemu-arm PROPERTIES EXCLUDE_FROM_ALL FALSE)

set(src "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(inc "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(gen ${CMAKE_CURRENT_BINARY_DIR}/gen)

configure_file(${src}/avp64/version.h.in
               ${gen}/avp64/version.h @ONLY)

add_library(avp64-psp STATIC
    ${src}/avp64/psp/core.cpp
    ${src}/avp64/psp/cpu.cpp
    ${src}/avp64/psp/systemc.cpp
)

target_compile_options(avp64-psp PRIVATE ${MWR_COMPILER_WARN_FLAGS})
target_include_directories(avp64-psp PUBLIC ${inc})
target_include_directories(avp64-psp PUBLIC ${gen})
target_include_directories(avp64-psp PUBLIC ${OCX_QEMU_ARM_HOME}/ocx/include)
target_link_libraries(avp64-psp PUBLIC ${CMAKE_DL_LIBS})
target_link_libraries(avp64-psp PUBLIC vcml)
set_target_properties(avp64-psp PROPERTIES CXX_STANDARD 17)
set_target_properties(avp64-psp PROPERTIES CXX_CLANG_TIDY "${AVP64_LINTER}")
set_target_properties(avp64-psp PROPERTIES VERSION "${AVP64_VERSION}")
set_target_properties(avp64-psp PROPERTIES SOVERSION "${AVP64_VERSION_MAJOR}")

if(NOT ${AVP64_LINTER} STREQUAL "")
    # -std=c++17 is not passed to the compiler if it is the comiler's default
    # this caused problems with clang-tidy
    set_target_properties(avp64-psp PROPERTIES CXX_STANDARD_REQUIRED ON)
endif()

install(TARGETS avp64-psp)
install(TARGETS ocx-qemu-arm DESTINATION lib)
install(DIRECTORY sw/ DESTINATION sw)
install(DIRECTORY ${inc}/ DESTINATION include)
install(DIRECTORY ${gen}/ DESTINATION include)

if(AVP64_TESTS)
    message(STATUS "Building AVP64 tests")
    enable_testing()
    add_subdirectory(tests)
endif()

if(AVP64_VP)
    add_executable(avp64
        ${src}/avp64/system.cpp
        ${src}/avp64/main.cpp
    )

    target_compile_options(avp64 PRIVATE ${MWR_COMPILER_WARN_FLAGS})
    target_include_directories(avp64 PRIVATE ${src})
    set_target_properties(avp64 PROPERTIES CXX_STANDARD 17)
    set_target_properties(avp64 PROPERTIES CXX_CLANG_TIDY "${AVP64_LINTER}")
    set_target_properties(avp64 PROPERTIES VERSION "${AVP64_VERSION}")
    set_target_properties(avp64 PROPERTIES SOVERSION "${AVP64_VERSION_MAJOR}")
    target_link_whole_archives(avp64 avp64-psp)
    target_link_whole_archives(avp64 vcml)

    if(NOT ${AVP64_LINTER} STREQUAL "")
        # -std=c++17 is not passed to the compiler if it is the comiler's default
        # this caused problems with clang-tidy
        set_target_properties(avp64 PROPERTIES CXX_STANDARD_REQUIRED ON)
    endif()

    install(TARGETS avp64)
endif()
