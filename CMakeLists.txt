cmake_minimum_required (VERSION 3.0)

project (tinypy)

FILE(GLOB SRCS *.h *.cpp)
SOURCE_GROUP("" FILES ${SRCS})
add_executable(tinypy ${SRCS})