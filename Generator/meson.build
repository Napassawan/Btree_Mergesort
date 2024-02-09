project('generator', 'cpp', 
	default_options : ['cpp_std=c++17'])

compiler = meson.get_compiler('cpp')

# --------------------------------------------------------------

system = build_machine.system()
message('System is ' + system)

deps = [dependency('openmp')]
if system != 'windows'
	deps += compiler.find_library('tbb')
endif

common = ['../Common/mygetopt.cpp']

executable('generator', 
	sources : ['generator_new.cpp'] + common,
	dependencies : deps)
