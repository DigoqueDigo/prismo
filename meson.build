project(
  'prismo',
  'cpp',
  version: '1.0.0',
  default_options: [
    'warning_level=3',
    'cpp_std=c++20',
    'optimization=0',
    'debug=true'
  ]
)

# Define include directories
# spdk_root = '/home/digo/Desktop/spdk'

inc = include_directories(
  'include',
  # join_paths(spdk_root, 'include'),
  # join_paths(spdk_root, 'build', 'include'),
)

# Define source files
src = files(
  'src/access/access.cpp',
  'src/access/random.cpp',
  'src/access/sequential.cpp',
  'src/access/zipfian.cpp',
  'src/engine/aio.cpp',
  'src/engine/posix.cpp',
  'src/engine/uring.cpp',
  'src/engine/utils.cpp',
  'src/generator/random.cpp',
  'src/logger/spdlog.cpp',
  'src/operation/barrier.cpp',
  'src/operation/constant.cpp',
  'src/operation/percentage.cpp',
  'src/operation/sequence.cpp',
  'src/parser/parser.cpp',
  'src/main.cpp',
)

# Get the C++ compiler
cc = meson.get_compiler('cpp')

# Find json dependency (nlohmann/json wrap)
json_dep = dependency('nlohmann_json', required: true)

# Find spdlog dependency (header-only)
spdlog_dep = dependency('spdlog', required: true)

# Find liburing dependency (header-only)
liburing_dep = dependency('liburing', required: true)

# Find libaio dependency (header-only)
libaio_dep = cc.find_library('aio', required: true)

# Find sdpk dependencies (statics libs)
# spdk_nvme_dep = dependency('spdk_nvme', required: true, static: true)
# spdk_env_dpdk_dep = dependency('spdk_env_dpdk', required: true, static: true)
# spdk_syslibs_dep = dependency('spdk_syslibs', required: true, static: true)

# Define spdk link arguments
# spdk_link_args = [
#   '-Wl,--whole-archive',
#   join_paths(spdk_root, 'build', 'lib', 'libspdk_nvme.a'),
#   '-Wl,--no-whole-archive',
# ]

# Common flags for all builds
common_flags = [
  '-Wpedantic',
  '-Wshadow',
  '-Wnon-virtual-dtor',
]

# Compiler-specific flags
if cc.get_id() == 'gcc' or cc.get_id() == 'clang'
  extra_flags = [
    '-Wconversion',
    '-Wsign-conversion',
  ]
endif

# add_project_arguments(common_flags + extra_flags, language: 'cpp')

prismo_exe = executable(
  'prismo',
  sources: src,
  include_directories : [inc],
  dependencies: [
    json_dep,
    spdlog_dep,
    liburing_dep,
    libaio_dep,
    # spdk_nvme_dep,
    # spdk_env_dpdk_dep,
    # spdk_syslibs_dep,
  ],
  # link_args: spdk_link_args,
  install : true,
)

# Define a test to run the prismo executable
test(
  'prismo_basic_test',
  prismo_exe,
  args : ['../config/config.json']
)