cmake_minimum_required(VERSION 3.23)
project(PoC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 加载源代码
aux_source_directory(src SRC)
aux_source_directory(lib LIB)

# 添加头文件路径
include_directories(include)

add_executable(main ${LIB} ${SRC}
        include/Quadruple.h)
