# Locate the Check library
check_dep = dependency('check')

# Define test sources
test_src = ['transform_tests.c']

# Build the test executable
test_exe = executable('transform_tests', test_src, link_with: lib, dependencies: check_dep)

# Add the test
test('unit test', test_exe)