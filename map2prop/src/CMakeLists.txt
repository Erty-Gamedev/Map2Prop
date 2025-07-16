add_executable(map2prop main.cpp)

target_link_libraries(map2prop Common Config Formats Geometry)
target_compile_features(map2prop PUBLIC cxx_std_20)
set_target_properties(map2prop PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${MAP2PROP_BIN_DIR}/debug"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${MAP2PROP_BIN_DIR}/debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${MAP2PROP_BIN_DIR}/release"
)
