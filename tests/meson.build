test('c_array_support',
  executable('test_c_array_support', 'test_c_array_support.cpp',
  dependencies : [c_array_support_dep])
)

test('c_array_compare',
  executable('test_c_array_compare', 'test_c_array_compare.cpp',
  dependencies : [c_array_support_dep])
)

test('c_array_assign',
  executable('test_c_array_assign', 'test_c_array_assign.cpp',
  dependencies : [c_array_support_dep])
)

test('zero_size_array',
  executable('test_zero_size_array', 'test_zero_size_array.cpp',
  dependencies : [c_array_support_dep],
  override_options : ['werror=true'])
)
