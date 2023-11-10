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
find_package(PythonInterp "3.9" REQUIRED)

add_library(test_main test_main.cpp)
target_link_libraries(test_main gtest)

macro(link_test test)
    target_include_directories(${test} PRIVATE ${inc} ${SYSTEMC_INCLUDE_DIRS})
    target_link_libraries(${test} test_main gtest vcml ${SYSTEMC_LIBRARIES} ${LIBELF_LIBRARIES} ${CMAKE_DL_LIBS})
    target_compile_options(${test} PRIVATE ${MWR_COMPILER_WARN_FLAGS})
    set_target_properties(${test} PROPERTIES CXX_CLANG_TIDY "${AVP64_LINTER}")
    add_test(NAME ${test} COMMAND ${test} ${CMAKE_CURRENT_SOURCE_DIR})
    set_tests_properties(${test} PROPERTIES TIMEOUT 5)
endmacro()

add_executable(arm64_cpu_test arm64_cpu_test.cpp
        ${src}/arm64_cpu.cpp)
link_test(arm64_cpu_test)

function(linux_boot nrcpu config timeout)
    set(name "linux-boot-${nrcpu}-cpus")
    set(config ${CMAKE_SOURCE_DIR}/config/${config})
    set(script ${CMAKE_CURRENT_BINARY_DIR}/${name}.py)
    get_target_property(ld_library_path ocx-qemu-arm BINARY_DIR)
    configure_file(linux_boot.py.in ${script})
    file(GENERATE OUTPUT ${script} INPUT ${script})
    add_test(NAME ${name} COMMAND ${PYTHON_EXECUTABLE} ${script})
    set_tests_properties(${name} PROPERTIES TIMEOUT ${timeout})
    set_tests_properties(${name} PROPERTIES ENVIRONMENT "${ENVVARS}")
endfunction()

linux_boot(1 arm64_linux_4_19_4-x1.cfg 600)

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE_UPPER)
if(BUILD_TYPE_UPPER STREQUAL RELEASE)
    linux_boot(2 arm64_linux_4_19_4-x2.cfg 600)
    linux_boot(4 arm64_linux_4_19_4-x4.cfg 600)
    linux_boot(8 arm64_linux_4_19_4-x8.cfg 600)
endif()
