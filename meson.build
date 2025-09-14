project(
  'dedisbench',
  'cpp',
  version: '1.0.0',
  default_options: [
    'warning_level=3',                                                # Equivalent to -Wall and -Wextra
    'cpp_std=c++17',                                                  # Equivalent to -std=c++17
    'optimization=0',                                                 # Default to -O0 for debug
    'debug=true'                                                      # Default to -g for debug
  ]
)

# Define include directories and source files
inc = include_directories('include')
src = files(
  'src/main.cpp',
)

# Get the C++ compiler
cpp = meson.get_compiler('cpp')

# Find Boost.Thread dependency
boost_dep = dependency(
  'boost',
  modules: ['thread', 'system'],
  required: true
)

# Common flags for all builds
common_flags = [
  '-Wpedantic',                                                     # Enforce strict standard compliance
  '-Wshadow',                                                       # Warn about variable shadowing
  '-Wnon-virtual-dtor',                                             # Warn about missing virtual destructors
]

# Compiler-specific flags
if cpp.get_id() == 'gcc' or cpp.get_id() == 'clang'
  extra_flags = [
    '-Wconversion',                                                 # Warn about implicit conversions
    '-Wsign-conversion',                                            # Warn about sign conversions
  ]
endif

# add_project_arguments(common_flags + extra_flags, language: 'cpp')

executable(
  'dedisbench',
  sources: src,
  include_directories : inc,
  dependencies: [boost_dep],
  install : true,
)