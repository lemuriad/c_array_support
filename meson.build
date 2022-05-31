project('c_array_support', 'cpp',
  default_options : ['cpp_std=c++20','warning_level=3'],
  version : '0.5'
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

test('c_array_support',
  executable('test_c_array_support', 'tests/test_array_support.cpp',
  dependencies : [c_array_support_dep])
)

test('array_compare',
  executable('test_array_compare', 'tests/test_array_compare.cpp',
  dependencies : [c_array_support_dep])
)

test('array_assign',
  executable('test_array_assign', 'tests/test_array_assign.cpp',
  dependencies : [c_array_support_dep])
)

test('zero_size_array',
  executable('test_zero_size_array', 'tests/test_zero_size_array.cpp',
  dependencies : [c_array_support_dep],
  override_options : ['werror=true'])
)
