cmake_minimum_required(VERSION 3.16)
project(kore-2022)

set(CMAKE_CXX_STANDARD 17)

if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
         "${CMAKE_BINARY_DIR}/conan.cmake"
         TLS_VERIFY ON)
endif ()

include("${CMAKE_BINARY_DIR}/conan.cmake")

conan_cmake_configure(REQUIRES pybind11/2.9.1 nlohmann_json/3.10.5 gtest/cci.20210126 benchmark/1.6.1
                      GENERATORS cmake_find_package)

conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE .
                    BUILD missing
                    REMOTE conancenter
                    SETTINGS ${settings})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}")

find_package(pybind11 REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(GTest REQUIRED)
find_package(benchmark REQUIRED)

set(TEST_LIBRARIES pybind11::pybind11 nlohmann_json::nlohmann_json GTest::GTest benchmark::benchmark)

file(GLOB_RECURSE AGENT_MAIN_FILES agents/v*/main.cpp)
foreach (AGENT_MAIN_FILE ${AGENT_MAIN_FILES})
    get_filename_component(AGENT_DIRECTORY "${AGENT_MAIN_FILE}" DIRECTORY)
    get_filename_component(AGENT_NAME "${AGENT_DIRECTORY}" NAME)

    if ("${AGENT_NAME}" STREQUAL tests)
        continue()
    endif ()

    file(GLOB_RECURSE AGENT_SOURCES "${AGENT_DIRECTORY}/*.cpp")
    list(FILTER AGENT_SOURCES EXCLUDE REGEX .*/tests/.*)
    pybind11_add_module("${AGENT_NAME}" ${AGENT_SOURCES})
    target_include_directories("${AGENT_NAME}" PRIVATE "${AGENT_DIRECTORY}")

    file(GLOB_RECURSE BENCHMARK_SOURCES "${AGENT_DIRECTORY}/*.cpp")
    list(REMOVE_ITEM BENCHMARK_SOURCES "${AGENT_DIRECTORY}/main.cpp")
    list(FILTER BENCHMARK_SOURCES EXCLUDE REGEX .*/tests/.*)
    list(APPEND BENCHMARK_SOURCES "${AGENT_DIRECTORY}/tests/benchmark.cpp")
    add_executable("${AGENT_NAME}_benchmark" ${BENCHMARK_SOURCES})
    target_include_directories("${AGENT_NAME}_benchmark" PRIVATE "${AGENT_DIRECTORY}")
    target_link_libraries("${AGENT_NAME}_benchmark" ${TEST_LIBRARIES})

    file(GLOB_RECURSE TEST_SOURCES "${AGENT_DIRECTORY}/*.cpp")
    list(REMOVE_ITEM TEST_SOURCES "${AGENT_DIRECTORY}/main.cpp")
    list(REMOVE_ITEM TEST_SOURCES "${AGENT_DIRECTORY}/tests/benchmark.cpp")
    add_executable("${AGENT_NAME}_test" ${TEST_SOURCES})
    target_include_directories("${AGENT_NAME}_test" PRIVATE "${AGENT_DIRECTORY}")
    target_link_libraries("${AGENT_NAME}_test" ${TEST_LIBRARIES})

    set_target_properties("${AGENT_NAME}" "${AGENT_NAME}_test" "${AGENT_NAME}_benchmark"
                          PROPERTIES
                          LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/agents/${AGENT_NAME}"
                          RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/agents/${AGENT_NAME}")

    add_custom_command(TARGET "${AGENT_NAME}" POST_BUILD
                       COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                       "${AGENT_DIRECTORY}/main.py"
                       "${CMAKE_BINARY_DIR}/agents/${AGENT_NAME}/main.py")

    add_custom_command(TARGET "${AGENT_NAME}" POST_BUILD
                       COMMAND "${CMAKE_COMMAND}" -E copy_directory
                       "${AGENT_DIRECTORY}/data"
                       "${CMAKE_BINARY_DIR}/agents/${AGENT_NAME}/data")

    add_custom_command(TARGET "${AGENT_NAME}_test" POST_BUILD
                       COMMAND "${CMAKE_COMMAND}" -E copy_directory
                       "${AGENT_DIRECTORY}/tests/data"
                       "${CMAKE_BINARY_DIR}/agents/${AGENT_NAME}/test-data")

    if ("${CMAKE_BUILD_TYPE}" STREQUAL Release)
        add_custom_command(TARGET "${AGENT_NAME}" POST_BUILD
                           COMMAND tar -C "${CMAKE_BINARY_DIR}/agents/${AGENT_NAME}" -czf
                           "${CMAKE_BINARY_DIR}/agents/${AGENT_NAME}/submission.tar.gz"
                           main.py "${AGENT_NAME}.cpython-37m-x86_64-linux-gnu.so" data)
    endif ()
endforeach ()
