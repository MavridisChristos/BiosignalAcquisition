cmake_minimum_required(VERSION 2.8.12.2)
project(cmbiosemi)

#Bring the headers
include_directories(include)

link_directories(.)

set(SOURCES src/cmbiosemi.cpp)
#file(GLOB SOURCES "src/*.cpp" )

add_executable(cmbiosemi ${SOURCES})
target_link_libraries(cmbiosemi -llabview_dll)
