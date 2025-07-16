cmake_minimum_required(VERSION 3.22)

include_directories(PUBLIC "${PROJECT_SOURCE_DIR}/vendor/doctest/doctest")

file(GLOB APP_HEADERS *.h)
file(GLOB APP_SRC *.cpp)

add_executable(tests ${APP_HEADERS} ${APP_SRC})

target_link_libraries(tests PRIVATE Formats Geometry)
target_compile_features(tests PRIVATE cxx_std_20)
set_target_properties(tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${MAP2PROP_BIN_DIR}/debug"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${MAP2PROP_BIN_DIR}/debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${MAP2PROP_BIN_DIR}/release"
)
