demi_src += files(
  'device.c',
  'context.c',
  'monitor.c',
  'enumerate.c',
)

demi_dep += cc.find_library('prop')
add_project_arguments('-D_NETBSD_SOURCE', language: 'c')
