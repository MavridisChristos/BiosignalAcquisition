cmake_minimum_required(VERSION 2.8.12.2)
project(cmliberty)

#Bring the headers
include_directories(include)

#set(SOURCES src/cmliberty.cpp src/liberty.cpp)
file(GLOB SOURCES "src/*.cpp" )

add_executable(cmliberty ${SOURCES})
target_link_libraries(cmliberty -lusb)
