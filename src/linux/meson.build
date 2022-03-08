demi_inc += include_directories('.')
demi_src += files(
  'device.c',
  'context.c',
  'monitor.c',
)

install_headers('demi_structs.h', subdir: 'demi')
add_project_arguments(
  '-D_GNU_SOURCE',
  '-DDEMI_MONITOR_NETLINK_GROUP=' + get_option('nl_group').to_string(),
  language: 'c')
