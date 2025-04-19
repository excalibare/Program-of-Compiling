cmake_minimum_required(VERSION 3.23)
project(PoC)

set(CMAKE_CXX_STANDARD 17)

aux_source_directory(src SRC)
aux_source_directory(lib LIB)

add_executable(main ${LIB} ${SRC}
        include/Utils.h)
include_directories(include)