project(
  'dedisbench',
  'cpp',
  version: '1.0.0',
)

inc = include_directories('include')

src = files(
  'src/access/access.cpp',
  'src/access/sequential.cpp',
  'src/main.cpp',
)

executable(
  'dedisbench',
  sources: src,
  include_directories : inc,
  install : true,
)