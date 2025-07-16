cmake_minimum_required(VERSION 3.22)

file(GLOB APP_HEADERS *.h)
file(GLOB APP_SRC *.cpp)

add_library(Formats ${APP_HEADERS} ${APP_SRC})

target_link_libraries(Formats Common Config Geometry)
target_compile_features(Formats PUBLIC cxx_std_20)
