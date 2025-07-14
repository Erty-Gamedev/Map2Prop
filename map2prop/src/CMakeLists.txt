add_executable(map2prop main.cpp)

target_link_libraries(map2prop Common Config Formats Geometry)
target_compile_features(map2prop PRIVATE cxx_std_20)
