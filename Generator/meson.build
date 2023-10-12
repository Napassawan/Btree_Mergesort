project('generator', 'cpp', 
	default_options : ['cpp_std=c++17'])

compiler = meson.get_compiler('cpp')

# --------------------------------------------------------------

omp = dependency('openmp')

common = ['../Common/mygetopt.cpp']

add_project_arguments('-g',
	language : 'cpp')

executable('generator', sources : [ 'generator.cpp' ],
	dependencies : omp)

executable('generator_new', 
	sources : ['generator_new.cpp'] + common,
	dependencies : omp)
