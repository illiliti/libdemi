demi_src += files(
  'device.c',
  'context.c',
  'monitor.c',
  'enumerate.c',
)

add_project_arguments(
  '-D_POSIX_C_SOURCE=200809L',
  '-DDEMI_MONITOR_NETLINK_GROUP=' + get_option('nl_group').to_string(),
  language: 'c')
