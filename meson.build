project(
  'dedisbench',
  'cpp',
  version: '1.0.0',
  default_options: [
    'warning_level=3',                                                # Equivalent to -Wall and -Wextra
    'cpp_std=c++20',                                                  # Equivalent to -std=c++20
    'optimization=0',                                                 # Default to -O0 for debug
    'debug=true'                                                      # Default to -g for debug
  ]
)

# Define include directories and source files
inc = include_directories('include')
src = files(
  'src/access/random.cpp',
  'src/access/sequential.cpp',
  'src/access/zipfian.cpp',
  'src/generator/block.cpp',
  'src/generator/constant.cpp',
  'src/generator/random.cpp',
  'src/operation/barrier.cpp',
  'src/operation/constant.cpp',
  'src/operation/percentage.cpp',
  'src/operation/sequence.cpp',
  'src/parser/access.cpp',
  'src/parser/engine.cpp',
  'src/parser/generator.cpp',
  'src/parser/logger.cpp',
  'src/parser/metric.cpp',
  'src/parser/operation.cpp',
  'src/main.cpp',
)

# Get the C++ compiler
cc = meson.get_compiler('cpp')

# Find Boost.Thread dependency
boost_dep = dependency(
  'boost',
  modules: ['thread', 'chrono'],
  required: true
)

# Find spdlog dependency (header-only)
spdlog_dep = dependency('spdlog', required: true)

# Find liburing dependency (header-only)
liburing_dep = dependency('liburing', required: true)

# Find json dependency (nlohmann/json wrap)
json_dep = dependency('nlohmann_json', required: true)

# Common flags for all builds
common_flags = [
  '-Wpedantic',                                                     # Enforce strict standard compliance
  '-Wshadow',                                                       # Warn about variable shadowing
  '-Wnon-virtual-dtor',                                             # Warn about missing virtual destructors
]

# Compiler-specific flags
if cc.get_id() == 'gcc' or cc.get_id() == 'clang'
  extra_flags = [
    '-Wconversion',                                                 # Warn about implicit conversions
    '-Wsign-conversion',                                            # Warn about sign conversions
  ]
endif

# add_project_arguments(common_flags + extra_flags, language: 'cpp')

dedisbench_exe = executable(
  'dedisbench',
  sources: src,
  include_directories : [inc],
  dependencies: [boost_dep, spdlog_dep, liburing_dep, json_dep],
  install : true,
)

# Define a test to run the dedisbench executable
test(
  'dedisbench_basic',
  dedisbench_exe,
  args : ['../config/config.json']
)