cmake_minimum_required(VERSION 3.12)
project(g2pk-dict VERSION 1.0.0 DESCRIPTION "Required dictionary for libg2pk")
include(GNUInstallDirs)
include(FindPython)
	
find_package(Python COMPONENTS Interpreter)

add_custom_command(
	OUTPUT ${CMAKE_BINARY_DIR}/english.txt
	COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/gen-english.py
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	COMMENT "Generating English Dictionary"
	VERBATIM)
add_custom_command(
	OUTPUT ${CMAKE_BINARY_DIR}/batchim.txt
	COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/gen-batchim.py
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	COMMENT "Generating Batchim table"
	VERBATIM)
	
add_custom_target(g2pk-dict ALL DEPENDS "${CMAKE_BINARY_DIR}/english.txt" "${CMAKE_BINARY_DIR}/batchim.txt")

install(FILES ${CMAKE_BINARY_DIR}/english.txt
	DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/libg2pk)
install(FILES ${CMAKE_BINARY_DIR}/batchim.txt
	DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/libg2pk)
install(FILES LICENSE.english
	DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/libg2pk)
install(FILES LICENSE.batchim
	DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/libg2pk)
