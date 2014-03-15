cmake_minimum_required(VERSION 2.8)
project(asuroflash)

find_package(PkgConfig REQUIRED)
pkg_search_module(gtkmm REQUIRED gtkmm-2.4)
pkg_search_module(gthread REQUIRED gthread-2.0)
include_directories(${gtkmm_INCLUDE_DIRS} ${gthread_INCLUDE_DIRS})

file(GLOB SRC *.cpp)
add_executable(asuroflash ${SRC})
target_link_libraries(asuroflash ${gthread_LDFLAGS} ${gtkmm_LDFLAGS})

install(TARGETS asuroflash RUNTIME DESTINATION bin)
