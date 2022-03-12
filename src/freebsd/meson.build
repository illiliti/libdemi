demi_inc += include_directories('.')
demi_src += files(
  'demi.c',
)

install_headers('demi_internal.h', subdir: 'demi')
add_project_arguments(
  '-DDEMI_MONITOR_DEVD_SOCKET="@0@"'.format(get_option('devd_socket')),
  language: 'c')
