cmake_minimum_required(VERSION 3.4)
project( "Feud" )

add_compile_definitions( ROUNDS_PATH="${CMAKE_SOURCE_DIR}/rounds/" )
add_executable( feud code/main.cpp )
