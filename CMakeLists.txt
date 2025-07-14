cmake_minimum_required(VERSION 3.22)

project(
    Map2Prop
    VERSION 2.0.0
    DESCRIPTION "Converts a .map/.rmf/.jmf/.ol or J.A.C.K .obj into GoldSrc .smd files for model creation"
    HOMEPAGE_URL "https://github.com/Erty-Gamedev/Map2Prop"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories(PUBLIC "common/src")
include_directories(PUBLIC "config/src")
include_directories(PUBLIC "formats/src")
include_directories(PUBLIC "geometry/src")
include_directories(PUBLIC "${PROJECT_SOURCE_DIR}/vendor/logging/src")

add_subdirectory(common/src)
add_subdirectory(config/src)
add_subdirectory(formats/src)
add_subdirectory(geometry/src)
add_subdirectory(map2prop/src)
