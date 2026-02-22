cmake_minimum_required(VERSION 3.10)

project(HTTP_SERVER C)

file(GLOB SRCS "*.c")

set(ALL_SRCS main.c ${SRCS})

add_executable(main.out ${ALL_SRCS})