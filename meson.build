project(
  'dedisbench',
  'cpp',
  version: '1.0.0',
)

inc = include_directories('include')

src = files(
  'src/offset/synthetic_strategy.cpp',
  'src/offset/distribution/offset_distribution.cpp',
  'src/offset/distribution/sequential_offset.cpp',
  'src/offset/distribution/random_offset.cpp',
  'src/offset/distribution/binomial_offset.cpp',
  'src/offset/distribution/zipf_offset.cpp',
  'src/operation/fixed_pattern.cpp',
  'src/operation/percentage_pattern.cpp',
  'src/main.cpp',
)

executable(
  'dedisbench',
  sources: src,
  include_directories : inc,
  install : true,
)