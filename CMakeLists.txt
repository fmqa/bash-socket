cmake_minimum_required(VERSION 3.9)
project(bashext-socket VERSION 1.0 DESCRIPTION "BASH socket extension" LANGUAGES C)
include(GNUInstallDirs)
set(DEFAULT_BUILD_TYPE "Release")
find_package(PkgConfig REQUIRED)
pkg_check_modules(BASH REQUIRED IMPORTED_TARGET bash)
add_library(bashext-socket SHARED
    src/accept.c
    src/socket.c
    src/alarm.c
    src/pause.c)
set_target_properties(bashext-socket PROPERTIES 
    VERSION ${PROJECT_VERSION} SOVERSION 1)
target_link_libraries(bashext-socket PUBLIC PkgConfig::BASH)
install(TARGETS bashext-socket
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
