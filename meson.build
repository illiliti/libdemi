project(
  'demi',
  'c',
  license: 'ISC',
  version: '0.0.0',
  meson_version: '>=0.49.0',
  default_options: [
    'c_std=c99',
    'warning_level=3',
  ],
)

cc = meson.get_compiler('c')

add_project_arguments(cc.get_supported_arguments([
    '-Wvla',
    '-Wgnu',
    '-Wundef',
    '-Walloca',
    '-Wshadow',
    '-Wformat=2',
    '-Wconversion',
    '-Warith-conversion',
    '-Wstrict-prototypes',
    '-Wmissing-prototypes',
    '-Wimplicit-fallthrough=2',
  ]),
  language: 'c',
)

demi_src = []
demi_inc = []
demi_dep = []

subdir('src')
subdir('include')

demi_lib = library(
  meson.project_name(),
  demi_src,
  include_directories: demi_inc,
  dependencies: demi_dep,
  version: meson.project_version(),
  install: true,
)

pkgconfig = import('pkgconfig')
pkgconfig.generate(
  demi_lib,
  subdirs: meson.project_name(),
  url: 'https://github.com/illiliti/libdemi',
  description: 'Device enumeration, monitoring and introspecting library',
)
