cmake_minimum_required (VERSION 3.1)
project (generator)

find_package(PkgConfig)
pkg_check_modules(sqlite3 sqlite3 REQUIRED)

include_directories(
  ${sqlite3_INCLUDE_DIR}
  vendor/json
  ../vendor/hkutil)

add_executable(generator generator.cpp main.cpp)
set_property(TARGET generator PROPERTY CXX_STANDARD 11)
set_property(TARGET generator PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(generator ${sqlite3_LIBRARIES})
