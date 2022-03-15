demi_inc += include_directories('.')
demi_dep += cc.find_library('prop')
demi_src += files('demi.c')

install_headers('demi_internal.h', subdir: 'demi')
add_project_arguments('-D_NETBSD_SOURCE', language: 'c')
