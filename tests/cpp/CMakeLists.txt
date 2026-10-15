cmake_minimum_required(VERSION 3.25)
project(pde_software_replication_core_test)

find_package(GTest CONFIG REQUIRED)

# Add all .cpp files in this directory as test sources
file(GLOB TEST_SOURCES *.cpp)
add_executable(pde_software_replication_core_test ${TEST_SOURCES})

# Acceptance output identifies the exact Actions source or local working tree.
set(PDE_SOURCE_REVISION "$ENV{GITHUB_SHA}")
if(NOT PDE_SOURCE_REVISION)
    find_package(Git QUIET)
    if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
        execute_process(COMMAND "${GIT_EXECUTABLE}" describe --always --dirty
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE PDE_SOURCE_REVISION OUTPUT_STRIP_TRAILING_WHITESPACE)
    else()
        set(PDE_SOURCE_REVISION "source-distribution")
    endif()
endif()
target_compile_definitions(pde_software_replication_core_test PRIVATE
    PDE_SOURCE_REVISION="${PDE_SOURCE_REVISION}"
    PDE_BUILD_ID="${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}-$<CONFIG>"
)

# Compile every public domain header in isolation as well as through the
# umbrella in multiple test translation units.
set(header_checks)
foreach(header Types Domain Grid Array Equation Tridiagonal Scheme/CrankNicholson1D PDE detail/Checked)
    string(REPLACE "/" "_" unit "${header}")
    set(source "${CMAKE_CURRENT_BINARY_DIR}/header_${unit}.cpp")
    file(WRITE "${source}" "#include <pde_software_replication_core/${header}.h>\n")
    list(APPEND header_checks "${source}")
endforeach()
add_library(pde_public_headers_compile OBJECT ${header_checks})
target_link_libraries(pde_public_headers_compile PRIVATE pde_software_replication_core_static)

target_include_directories(pde_software_replication_core_test PRIVATE
    ${CMAKE_SOURCE_DIR}/cpp/include
)

target_link_libraries(pde_software_replication_core_test PRIVATE
    pde_software_replication_core_static
    GTest::gtest
    GTest::gtest_main
    # GTest::gmock
    # GTest::gmock_main
)

add_test(NAME pde_software_replication_core_test COMMAND pde_software_replication_core_test)
