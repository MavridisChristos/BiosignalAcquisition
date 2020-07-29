cmake_minimum_required(VERSION 2.8.12.2)
project(cmdelsys)

#Bring the headers
include_directories(include)

#set(SOURCES src/cmdelsys.cpp)
file(GLOB SOURCES "src/*.cpp" )

add_executable(cmdelsys ${SOURCES})
target_link_libraries(cmdelsys -lcomedi -lm)
