cmake_minimum_required(VERSION 3.20)

set(APP_NAME generator)

project(${APP_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenMP REQUIRED)

set(COMMON_SOURCES
	../Common/mygetopt.cpp
)
set(GENERATOR_SOURCES
	generator_new.cpp
)

add_executable(${APP_NAME} ${COMMON_SOURCES} ${GENERATOR_SOURCES})

target_link_libraries(${APP_NAME} PUBLIC OpenMP::OpenMP_CXX)
