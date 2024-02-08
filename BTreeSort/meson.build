project('btreesort', 'cpp', 
	default_options : ['cpp_std=c++17'])

compiler = meson.get_compiler('cpp')

# --------------------------------------------------------------

system = build_machine.system()
message('System is ' + system)

incs = [include_directories('../External')]

deps = [dependency('openmp')]
if system == 'windows'
	deps += compiler.find_library('ntdll')
else
	deps += compiler.find_library('tbb')
endif

srcs_common = ['../Common/mygetopt.cpp', '../Common/reader.cpp']
srcs_main = ['timer.cpp', 'main.cpp']

executable('btreesort', 
	sources : srcs_common + srcs_main,
	include_directories: incs,
	dependencies : deps)

