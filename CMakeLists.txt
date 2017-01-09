cmake_minimum_required (VERSION 3.1)
project (cadence)

find_package(PkgConfig)
pkg_check_modules(sqlite3 sqlite3 REQUIRED)

add_subdirectory(vendor/libtwittercpp)
add_subdirectory(vendor/yaml-cpp EXCLUDE_FROM_ALL)

include_directories(${sqlite3_INCLUDE_DIR} vendor/libtwittercpp/src vendor/yaml-cpp/include)
add_executable(cadence cadence.cpp)
set_property(TARGET cadence PROPERTY CXX_STANDARD 11)
set_property(TARGET cadence PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(cadence ${sqlite3_LIBRARIES} twitter++ yaml-cpp)
