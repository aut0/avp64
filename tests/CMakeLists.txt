##############################################################################
#                                                                            #
# Copyright 2024 Lukas Jünger, Nils Bosbach                                  #
#                                                                            #
# This software is licensed under the MIT license.                           #
# A copy of the license can be found in the LICENSE file at the root         #
# of the source tree.                                                        #
#                                                                            #
##############################################################################

set(CMAKE_EXE_LINKER_FLAGS -rdynamic)

get_target_property(ld_library_path ocx-qemu-arm BINARY_DIR)

add_library(test_main test_main.cpp)
target_link_libraries(test_main PUBLIC gtest)
target_link_libraries(test_main PUBLIC avp64-psp)
target_link_libraries(test_main PUBLIC vcml)
set(PYTHON_PACKAGES_HOME "${CMAKE_CURRENT_BINARY_DIR}/python")

macro(new_test test)
    add_executable(${test} ${test}.cpp)
    target_include_directories(${test} PRIVATE ${inc} ${SYSTEMC_INCLUDE_DIRS})
    target_link_libraries(${test} test_main)
    target_compile_options(${test} PRIVATE ${MWR_COMPILER_WARN_FLAGS})
    add_test(NAME ${test} COMMAND ${test} ${CMAKE_CURRENT_SOURCE_DIR})
    set_tests_properties(${test} PROPERTIES TIMEOUT 5)
    set_tests_properties(${test} PROPERTIES ENVIRONMENT LD_LIBRARY_PATH=${ld_library_path}:$ENV{LD_LIBRARY_PATH})
endmacro()

new_test(arm64_core_test)
new_test(arm64_reset_test)
if (AVP64_BUILD_VP)
    function(linux name input_script nrcpu config timeout)
        set(config ${CMAKE_SOURCE_DIR}/sw/${config})
        set(script ${CMAKE_CURRENT_BINARY_DIR}/${name}.py)
        configure_file(${input_script} ${script})
        file(GENERATE OUTPUT ${script} INPUT ${script})
        add_test(NAME ${name} COMMAND python3 ${script})
        set_tests_properties(${name} PROPERTIES TIMEOUT ${timeout})
        set_tests_properties(${name} PROPERTIES ENVIRONMENT LD_LIBRARY_PATH=${ld_library_path}:$ENV{LD_LIBRARY_PATH})
    endfunction()

    function(linux_boot nrcpu config timeout)
        linux("linux-boot-${nrcpu}-cpus" linux_boot.py.in ${nrcpu} ${config} ${timeout})
    endfunction()

    function(linux_screenshot nrcpu config timeout)
        set(ref ${CMAKE_SOURCE_DIR}/tests/asset/test_card_reference.bmp)
        set(name "linux-screenshot-${nrcpu}-cpus")
        linux(${name} linux_screenshot.py.in ${nrcpu} ${config} ${timeout})

        # set python path
        get_test_property(${name} ENVIRONMENT test_env)
        set(test_env ${test_env};PYTHONPATH=${PYTHON_PACKAGES_HOME}:$ENV{PYTHONPATH})
        set_tests_properties(${name} PROPERTIES ENVIRONMENT "${test_env}")
    endfunction()

    linux_boot(1 buildroot_6_5_6-x1.cfg 600)

    string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE_UPPER)
    if(BUILD_TYPE_UPPER STREQUAL RELEASE)
        linux_boot(2 buildroot_6_5_6-x2.cfg 600)
        linux_boot(4 buildroot_6_5_6-x4.cfg 600)
        linux_boot(8 buildroot_6_5_6-x8.cfg 600)

        # screenshot
        set(PYVP_HOME ${PYTHON_PACKAGES_HOME}/pyvp)
        set(PYVP_REPO "machineware-gmbh/pyvp")
        if(NOT EXISTS ${PYVP_HOME})
            clone_github_repo(pyvp ${PYVP_REPO})
        endif()
        linux_screenshot(1 buildroot_6_5_6-x1.cfg 600)
    endif()
endif()
