cmake_minimum_required(VERSION 3.22)

file(GLOB APP_HEADERS *.h)
file(GLOB APP_SRC *.cpp)

add_library(Config ${APP_HEADERS} ${APP_SRC})

target_link_libraries(Config Common)
target_compile_features(Config PUBLIC cxx_std_20)
