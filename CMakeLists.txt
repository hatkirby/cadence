cmake_minimum_required (VERSION 3.1)
project (cadence)

find_package(PkgConfig)
pkg_check_modules(sqlite3 sqlite3 REQUIRED)
pkg_check_modules(yaml-cpp yaml-cpp REQUIRED)

add_subdirectory(vendor/libtwittercpp)

include_directories(
  ${sqlite3_INCLUDE_DIR}
  vendor/libtwittercpp/src
  ${yaml-cpp_INCLUDE_DIRS}
  vendor/hkutil)

add_executable(cadence cadence.cpp)
set_property(TARGET cadence PROPERTY CXX_STANDARD 11)
set_property(TARGET cadence PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(cadence ${sqlite3_LIBRARIES} twitter++ ${yaml-cpp_LIBRARIES})
