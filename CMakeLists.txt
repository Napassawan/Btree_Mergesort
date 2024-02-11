cmake_minimum_required(VERSION 3.10)

set(APP_NAME btree_msort)

project(${APP_NAME})

set(BTREESORT_V_MAJOR 0)
set(BTREESORT_V_MINOR 1)
set(BTREESORT_V_BUILD 0)
set(BTREESORT_VERSION "${BTREESORT_V_MAJOR}.${BTREESORT_V_MINOR}.${BTREESORT_V_BUILD}")

# --------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(BUILD_GENERATOR "Build generator" ON)
option(BUILD_BENCHMARK "Build benchmark" ON)
option(BUILD_VISUALIZER "Build visualizer (requires glfw)" OFF)

# --------------------------------------------------------------

find_package(OpenMP REQUIRED)
if (NOT WIN32)
	find_package(TBB)
endif()

set(COMMON_SRCS
	common/mygetopt.cpp
	common/reader.cpp
)

# --------------------------------------------------------------

if (BUILD_GENERATOR)
	set(GENERATOR_NAME generator)
	
	set(GENERATOR_SRCS
		generator/generator_new.cpp
	)
	
	add_executable(${GENERATOR_NAME} ${COMMON_SRCS} ${GENERATOR_SRCS})
	
	target_link_libraries(${GENERATOR_NAME} PUBLIC OpenMP::OpenMP_CXX)
	
	if (NOT WIN32 AND TBB_FOUND)
		target_link_libraries(${GENERATOR_NAME} PUBLIC TBB::tbb)
	endif()
endif()

# --------------------------------------------------------------

if (BUILD_BENCHMARK)
	set(BENCHMARK_NAME perf_bench)
	
	set(BENCHMARK_SRCS
		benchmark/timer.cpp
		benchmark/main.cpp
	)
	
	add_executable(${BENCHMARK_NAME} ${COMMON_SRCS} ${BENCHMARK_SRCS})
	
	include_directories(${BENCHMARK_NAME}
		external
		btree-sort
	)
	
	target_link_libraries(${BENCHMARK_NAME} PUBLIC OpenMP::OpenMP_CXX)
	
	if (WIN32)
		target_link_libraries(${BENCHMARK_NAME} PUBLIC ntdll)
	elseif (TBB_FOUND)
		target_link_libraries(${BENCHMARK_NAME} PUBLIC TBB::tbb)
	endif()
endif()

# --------------------------------------------------------------

if (BUILD_VISUALIZER)
	set(VISUALIZE_NAME visualize)
	
	option(CUSTOM_GLFW "Custom glfw location, else find_package is used" OFF)
	
	find_package(OpenGL REQUIRED)
	
	if(NOT CUSTOM_GLFW)
		find_package(glfw3 REQUIRED)
	else()
		if(NOT DEFINED glfw3_INCLUDE_DIR OR NOT DEFINED glfw3_LIBRARY)
			message(FATAL_ERROR "Please manually set glfw3_INCLUDE_DIR and glfw3_LIBRARY")
		endif()
	endif()
	
	set(VISUALIZER_SRCS
		visualize/main.cpp
	)
	
	set(IMGUI_SRCS
		visualize/imgui/imgui.cpp
		visualize/imgui/imgui_draw.cpp
		visualize/imgui/imgui_tables.cpp
		visualize/imgui/imgui_widgets.cpp
		visualize/imgui/backends/imgui_impl_glfw.cpp
		visualize/imgui/backends/imgui_impl_opengl3.cpp
	)
	
	add_executable(${VISUALIZE_NAME} ${COMMON_SRCS} ${VISUALIZER_SRCS} ${IMGUI_SRCS})
	
	include_directories(${VISUALIZE_NAME} 
		external
		btree-sort
		visualize/imgui
		${glfw3_INCLUDE_DIR}
	)
	
	target_link_libraries(${VISUALIZE_NAME} PUBLIC OpenMP::OpenMP_CXX OpenGL::GL)
	target_link_libraries(${VISUALIZE_NAME} PUBLIC ${glfw3_LIBRARY})
	
	if (WIN32)
		target_link_libraries(${VISUALIZE_NAME} PUBLIC ntdll gdi32 shell32)
	endif()
endif()
