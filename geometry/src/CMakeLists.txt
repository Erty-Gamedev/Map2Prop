cmake_minimum_required(VERSION 3.22)

file(GLOB APP_HEADERS *.h)
file(GLOB APP_SRC *.cpp)

add_library(Geometry ${APP_HEADERS} ${APP_SRC})

target_compile_features(Geometry PUBLIC cxx_std_20)
