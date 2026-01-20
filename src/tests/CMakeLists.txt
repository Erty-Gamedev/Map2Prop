cmake_minimum_required(VERSION 3.22)

file(GLOB TESTS_SRC test_*.cpp)

add_executable(tests main.cpp ${TESTS_SRC})


target_include_directories(tests PUBLIC
    "${PROJECT_SOURCE_DIR}/vendor/doctest/doctest"
)
target_link_libraries(tests PRIVATE Formats Geometry)

target_compile_features(tests PRIVATE cxx_std_20)
set_target_properties(tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${MAP2PROP_BIN_DIR}/debug"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${MAP2PROP_BIN_DIR}/debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${MAP2PROP_BIN_DIR}/release"
)
