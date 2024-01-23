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

srcs_common = ['../Common/mygetopt.cpp', '../Common/reader.cpp']
srcs_main = ['timer.cpp', 'main.cpp']

executable('msort', 
	sources : srcs_common + srcs_main,
	include_directories: incs,
	dependencies : deps)

