demi_inc += include_directories('.')
demi_src += files(
  'device.c',
  'context.c',
  'monitor.c',
  'enumerate.c',
)

install_headers('demi_structs.h', subdir: 'demi')
add_project_arguments(
  '-DDEMI_MONITOR_DEVD_SOCKET="@0@"'.format(get_option('devd_socket')),
  language: 'c')
