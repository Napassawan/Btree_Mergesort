cmake_minimum_required(VERSION 3.10)

set(APP_NAME btreesort)

project(${APP_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenMP REQUIRED)
if (NOT WIN32)
	find_package(TBB)
endif()

set(COMMON_SOURCES
	../Common/mygetopt.cpp
	../Common/reader.cpp
)
set(BTREESORT_SOURCES
	timer.cpp
	main.cpp
)

add_executable(${APP_NAME} ${COMMON_SOURCES} ${BTREESORT_SOURCES})

include_directories(${APP_NAME} ../External)
target_link_libraries(${APP_NAME} PUBLIC OpenMP::OpenMP_CXX)

if (WIN32)
	target_link_libraries(${APP_NAME} PUBLIC ntdll)
elseif (TBB_FOUND)
	target_link_libraries(${APP_NAME} PUBLIC TBB::tbb)
endif()
