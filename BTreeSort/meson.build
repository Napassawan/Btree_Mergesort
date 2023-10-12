project('msort', 'cpp', 
	default_options : ['cpp_std=c++17'])

compiler = meson.get_compiler('cpp')

# --------------------------------------------------------------

system = build_machine.system()

message('System is ' + system)

deps = [dependency('openmp')]
if system == 'windows'
	deps += compiler.find_library('ntdll')
endif

common = ['../Common/mygetopt.cpp']

add_project_arguments('-g',
	language : 'cpp')

executable('msort', 
	sources : common + ['main.cpp', 'timer.cpp'],
	dependencies : deps)

