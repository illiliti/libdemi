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
    '-Walloca',
    '-Wstrict-prototypes',
    '-Wmissing-prototypes',
  ]),
  language: 'c',
)

demi_src = []
demi_dep = []

subdir('src')
subdir('include')

demi_sym = 'libdemi.sym'
demi_lib = library(
  meson.project_name(),
  demi_src,
  include_directories: include_directories('include'),
  dependencies: demi_dep,
  version: meson.project_version(),
  link_args: '-Wl,--version-script,' + meson.current_source_dir() / demi_sym,
  link_depends: demi_sym,
  install: true,
)

pkgconfig = import('pkgconfig')
pkgconfig.generate(
  demi_lib,
  url: 'https://github.com/illiliti/libdemi',
  description: 'Device enumeration, monitoring and introspecting library',
)
