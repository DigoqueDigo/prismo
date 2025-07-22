project(
  'dedisbench',
  'cpp',
  version: '1.0.0',
)

inc = include_directories('include')

src = files(
  'src/access/unreal.cpp',
  'src/access/distribution/distribution.cpp',
  'src/access/distribution/sequential.cpp',
  'src/access/distribution/random.cpp',
  'src/access/distribution/binomial.cpp',
  'src/main.cpp',
)

executable(
  'dedisbench',
  sources: src,
  include_directories : inc,
  install : true,
)