project('c_array_support', 'cpp',
  default_options : ['cpp_std=c++20','warning_level=3'],
  version : 'v1.0'
)

if meson.get_compiler('cpp').get_id() == 'msvc'
  extra_args = ['/Zc:preprocessor']
else
  extra_args = []
endif

c_array_support_dep = declare_dependency(
  include_directories : include_directories('.'),
  compile_args : extra_args
)

if get_option('tests').disable_auto_if(meson.is_subproject()).allowed()
  subdir('tests')
endif
