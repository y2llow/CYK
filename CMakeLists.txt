cmake_minimum_required(VERSION 3.30)
project(MB_ProgrammeerOpdrachten)

set(CMAKE_CXX_STANDARD 20)

add_executable(MB_ProgrammeerOpdrachten main.cpp
        src/CFG.cpp
        src/PDA.cpp
)
