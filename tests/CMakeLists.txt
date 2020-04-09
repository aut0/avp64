##############################################################################
#                                                                            #
# Copyright 2020 Lukas Jünger                                                #
#                                                                            #
# This software is licensed under the MIT license.                           #
# A copy of the license can be found in the LICENSE file at the root         #
# of the source tree.                                                        #
#                                                                            #
##############################################################################

set(CMAKE_EXE_LINKER_FLAGS -rdynamic)

add_library(test_main test_main.cpp)
target_link_libraries(test_main gtest)

macro(link_test test)
    target_include_directories(${test} PRIVATE ${inc} ${SYSTEMC_INCLUDE_DIRS})
    target_link_libraries(${test} test_main gtest vcml ${SYSTEMC_LIBRARIES} ${LIBELF_LIBRARIES} dl)
    add_test(NAME ${test} COMMAND ${test} ${CMAKE_CURRENT_SOURCE_DIR})
    set_tests_properties(${test} PROPERTIES TIMEOUT 5)
endmacro()

add_executable(arm64_cpu_test arm64_cpu_test.cpp
        ${src}/arm64_cpu.cpp)
link_test(arm64_cpu_test)
