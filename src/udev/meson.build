demi_inc += include_directories('.')
demi_dep += dependency('libudev')
demi_src += files(
  'device.c',
  'context.c',
  'monitor.c',
)

install_headers('demi_structs.h', subdir: 'demi')
add_project_arguments('-D_POSIX_C_SOURCE=200809L', language: 'c')
