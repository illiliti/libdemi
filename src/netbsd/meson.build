demi_inc += include_directories('.')
demi_dep += cc.find_library('prop')
demi_src += files(
  'device.c',
  'context.c',
  'monitor.c',
  'enumerate.c',
)

install_headers('demi_structs.h', subdir: 'demi')
add_project_arguments('-D_NETBSD_SOURCE', language: 'c')
