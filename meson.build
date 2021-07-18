project('c_array_support', 'cpp',
  default_options : ['cpp_std=c++2a','warning_level=3'],
  version : '0.1'
)

c_array_support_dep = declare_dependency(
  include_directories : include_directories('.')
)
    
test('test c_array_support',
  executable('test_c_array_support', 'tests/test_array_support.cpp')
)

test('test array_compare',
  executable('test_array_compare', 'tests/test_array_compare.cpp')
)

test('test array_assign',
  executable('test_array_assign', 'tests/test_array_assign.cpp')
)
