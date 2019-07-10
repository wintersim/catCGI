cmake_minimum_required(VERSION 3.7)
project(catAjax C)

set(CMAKE_C_STANDARD 99)

add_executable(catAjax main.c database.c database.h logger.c logger.h)


link_directories(/usr/local/lib)

target_link_libraries(catAjax kcgi m sqlite3 sodium z)
