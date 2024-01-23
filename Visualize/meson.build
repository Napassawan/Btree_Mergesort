project('msort-visual', 'cpp', 
	default_options : ['cpp_std=c++17'])

cwd = meson.current_source_dir()
compiler = meson.get_compiler('cpp')

# --------------------------------------------------------------

system = build_machine.system()

message('System is ' + system)

incs = [
	include_directories('../External'),
	include_directories('imgui'),
]

deps = [dependency('openmp'),
	compiler.find_library('glfw3', dirs: cwd + '/lib'),
	compiler.find_library('opengl32')]

if system == 'windows'
	deps += compiler.find_library('ntdll')
	deps += compiler.find_library('gdi32')
	deps += compiler.find_library('shell32')
endif

srcs_common = ['../Common/mygetopt.cpp', '../Common/reader.cpp']
srcs_imgui =  ['imgui/imgui.cpp', 'imgui/imgui_draw.cpp', 
	'imgui/imgui_tables.cpp', 'imgui/imgui_widgets.cpp', 
	'imgui/backends/imgui_impl_glfw.cpp', 'imgui/backends/imgui_impl_opengl3.cpp']

srcs_main = ['main.cpp']

executable('msort-visual', 
	sources : srcs_common + srcs_imgui + srcs_main,
	include_directories: incs,
	dependencies : deps)
