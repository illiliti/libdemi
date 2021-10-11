demi_src += files(
  'device.c',
  'context.c',
  'monitor.c',
  'enumerate.c',
)

demi_dep += dependency('libudev')
add_project_arguments('-D_POSIX_C_SOURCE=200809L', language: 'c')
