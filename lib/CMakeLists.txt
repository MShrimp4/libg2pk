cmake_minimum_required(VERSION 3.9)
project(g2pk VERSION 1.0.0 DESCRIPTION "C++ implementation of G2Pk")
include(GNUInstallDirs)#find_package(GNUInstallDirs REQUIRED)
include(FindPkgConfig)#find_package(PkgConfig REQUIRED)
add_library(g2pk SHARED libg2pk.cpp)
set_target_properties(g2pk PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER libg2pk.h)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
pkg_check_modules (LIBHANGUL libhangul REQUIRED)
pkg_check_modules (AHOCORASICK libahocorasick)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
target_link_libraries(g2pk ${LIBHANGUL_LIBRARIES} mecab ahocorasick)
target_include_directories(g2pk PRIVATE .)
target_include_directories(g2pk PRIVATE ${LIBHANGUL_INCLUDE_DIRS})
target_compile_options(g2pk PRIVATE ${LIBHANGUL_CFLAGS_OTHER})

configure_file(g2pk.pc.in g2pk.pc @ONLY)
configure_file(libg2pk_conf.h.in libg2pk_conf.h)

install(TARGETS g2pk
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_BINARY_DIR}/g2pk.pc
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
