cmake_minimum_required(VERSION 3.22)

set(MAP2PROP_NAME           "Map2Prop")
set(MAP2PROP_VMAJOR         "2")
set(MAP2PROP_VMINOR         "2")
set(MAP2PROP_VPATCH         "2")
set(MAP2PROP_VERSION        "${MAP2PROP_VMAJOR}.${MAP2PROP_VMINOR}.${MAP2PROP_VPATCH}")
add_compile_definitions(MAP2PROP_VERSION="${MAP2PROP_VERSION}")

project(
    Map2Prop
    VERSION "${MAP2PROP_VERSION}"
    DESCRIPTION "Converts a .map/.rmf/.jmf/.ol or J.A.C.K .obj into GoldSrc .smd files for model creation"
    HOMEPAGE_URL "https://github.com/Erty-Gamedev/Map2Prop"
    LANGUAGES CXX
)

set(MAP2PROP_BIN_DIR "${PROJECT_SOURCE_DIR}/bin")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

include_directories(PUBLIC
    "src/common"
    "src/config"
    "src/formats"
    "src/geometry"
    "${PROJECT_SOURCE_DIR}/vendor/logging/src"
)

add_subdirectory("src/common")
add_subdirectory("src/config")
add_subdirectory("src/formats")
add_subdirectory("src/geometry")
add_subdirectory("src/map2prop")
add_subdirectory("src/tests")
