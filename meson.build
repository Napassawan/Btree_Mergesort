project('btree_msort', 'cpp', 
	default_options : ['cpp_std=c++17'])

cwd = meson.current_source_dir()
compiler = meson.get_compiler('cpp')

dir_incs = get_option('includedir')
dir_libs = get_option('libdir')

system = build_machine.system()
message('System is ' + system)

build_generator = get_option('build_generator')
build_benchmark = get_option('build_benchmark')
build_visualizer = get_option('build_visualizer')

# --------------------------------------------------------------

dep_omp = dependency('openmp')
dep_tbb = declare_dependency(
	dependencies : compiler.find_library('tbb',		required : false),
)

srcs_common = ['common/mygetopt.cpp', 'common/reader.cpp']

# --------------------------------------------------------------

if build_generator
	message('build_generator enabled')
	
	deps = [dep_omp]
	if system != 'windows'
		deps += dep_tbb
	endif

	srcs_main = ['generator/generator_new.cpp']

	executable('generator', 
		sources : srcs_main + srcs_common,
		dependencies : deps)
endif

# --------------------------------------------------------------

if build_benchmark
	message('build_benchmark enabled')
	
	incs = [
		include_directories('external'),
		include_directories('btree-sort'),
	]

	deps = [dep_omp]
	if system == 'windows'
		deps += compiler.find_library('ntdll')
	else
		deps += dep_tbb
	endif

	srcs_main = ['Benchmark/timer.cpp', 'Benchmark/main.cpp']

	executable('perf_bench', 
		sources : srcs_main + srcs_common,
		include_directories: incs,
		dependencies : deps)
endif

# --------------------------------------------------------------

if build_visualizer
	message('build_visualizer enabled')
	
	incs = [
		include_directories('external'),
		include_directories('btree-sort'),
		include_directories('visualize/imgui'),
	]
	
	dep_glfw = declare_dependency(
		include_directories: dir_incs,
		dependencies: compiler.find_library('glfw3', 	dirs : dir_libs),
	)
	deps = [
		dep_omp,
		compiler.find_library('opengl32', 	dirs : dir_libs),
		dep_glfw
	]

	if system == 'windows'
		deps += compiler.find_library('ntdll')
		deps += compiler.find_library('gdi32')
		deps += compiler.find_library('shell32')
	endif
	
	srcs_imgui =  [
		'visualize/imgui/imgui.cpp', 'visualize/imgui/imgui_draw.cpp', 
		'visualize/imgui/imgui_tables.cpp', 'visualize/imgui/imgui_widgets.cpp', 
		'visualize/imgui/backends/imgui_impl_glfw.cpp', 
		'visualize/imgui/backends/imgui_impl_opengl3.cpp'
	]

	srcs_main = ['visualize/main.cpp']
	
	executable('visualize', 
		sources : srcs_main + srcs_common + srcs_imgui,
		include_directories: incs,
		dependencies : deps)
endif
