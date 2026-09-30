include(GoogleTest)
add_library(xgct_dma_example STATIC ../examples/circular_dma.c)
set_target_properties(xgct_dma_example PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
target_link_libraries(xgct_dma_example PUBLIC xgct::ring_buffer)
xgct_warnings(xgct_dma_example)
if(XGCT_ENABLE_COVERAGE)
    target_compile_options(xgct_dma_example PRIVATE --coverage -O0 -g)
endif()
add_executable(xgct_tests unit/test_migration.cpp unit/test_containers.cpp unit/test_dma_cycle.cpp unit/test_boundaries.cpp)
set_target_properties(xgct_tests PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
target_link_libraries(xgct_tests PRIVATE xgct::list xgct::hash xgct::bitset xgct::ring_buffer xgct_dma_example GTest::gtest_main)
xgct_warnings(xgct_tests)
foreach(component list hash bitset ring_buffer version)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/header_${component}.c" "#include <xgen/containers/${component}.h>\ntypedef int header_check_t;\n")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/header_${component}.cpp" "#include <xgen/containers/${component}.h>\n")
    add_library(xgct_header_${component} OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/header_${component}.c"
        "${CMAKE_CURRENT_BINARY_DIR}/header_${component}.cpp")
    set_target_properties(xgct_header_${component} PROPERTIES
        C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS NO
        CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
    target_link_libraries(xgct_header_${component} PRIVATE xgct::list xgs::status)
    xgct_warnings(xgct_header_${component})
endforeach()
gtest_discover_tests(xgct_tests DISCOVERY_MODE POST_BUILD PROPERTIES LABELS "unit" TIMEOUT 30)
foreach(test_case source_list source_hash source_bitset source_ring_buffer
        installed_list installed_hash installed_bitset installed_ring_buffer
        installed_wrong_version installed_unknown installed_bad_abi
        installed_bad_type installed_optional installed_missing
        supplied_good supplied_bad_abi supplied_incomplete)
    add_test(NAME xgct_${test_case} COMMAND "${CMAKE_COMMAND}"
        "-DCASE=${test_case}" "-DSOURCE=${PROJECT_SOURCE_DIR}"
        "-DBINARY=${PROJECT_BINARY_DIR}" "-DSTATUS_SOURCE=${XGCT_STATUS_SOURCE_DIR}"
        "-DDEPENDENCY_PREFIX=${CMAKE_PREFIX_PATH}"
        "-DCC=${CMAKE_C_COMPILER}" "-DGENERATOR=${CMAKE_GENERATOR}"
        "-DPLATFORM=${CMAKE_GENERATOR_PLATFORM}" "-DTOOLSET=${CMAKE_GENERATOR_TOOLSET}"
        "-DCONFIG=$<CONFIG>" -P "${CMAKE_CURRENT_SOURCE_DIR}/integration/check_consumer.cmake")
    set_tests_properties(xgct_${test_case} PROPERTIES LABELS "integration" TIMEOUT 120)
endforeach()
