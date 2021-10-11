demi_src += files(
  'device.c',
  'context.c',
  'monitor.c',
  'enumerate.c',
)

demi_dep += meson.get_compiler('c').find_library('prop')
add_project_arguments('-D_NETBSD_SOURCE', language: 'c')
