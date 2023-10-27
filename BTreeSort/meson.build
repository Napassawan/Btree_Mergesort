project('msort', 'cpp', 
	default_options : ['cpp_std=c++17'])

compiler = meson.get_compiler('cpp')

# --------------------------------------------------------------

system = build_machine.system()

message('System is ' + system)

incs = [include_directories('../External')]

deps = [dependency('openmp')]
if system == 'windows'
	deps += compiler.find_library('ntdll')
endif

common = ['../Common/mygetopt.cpp', '../Common/reader.cpp']

add_project_arguments('-g',
	language : 'cpp')

executable('msort', 
	sources : common + ['main.cpp', 'timer.cpp', 'sort.cpp'],
	include_directories: incs,
	dependencies : deps)

