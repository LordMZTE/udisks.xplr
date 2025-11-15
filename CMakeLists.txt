cmake_minimum_required(VERSION 4.0)

project(udisks_xplr VERSION 0.0.0
                    LANGUAGES CXX
                    DESCRIPTION "xplr plugin for udisks2")

add_library(udisks_xplr SHARED
  src/Manager.cxx
  src/main.cxx
  src/udisks.cxx
  src/util.cxx
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(LUAJIT_PKG REQUIRED IMPORTED_TARGET GLOBAL luajit)
pkg_check_modules(UDISKS2_PKG REQUIRED IMPORTED_TARGET GLOBAL udisks2)

target_link_libraries(udisks_xplr PRIVATE
  PkgConfig::LUAJIT_PKG
  PkgConfig::UDISKS2_PKG)

install(TARGETS udisks_xplr
  DESTINATION udisks/)

install(FILES init.lua DESTINATION udisks/)
