if get_option('udev')
  subdir('udev')
elif host_machine.system() == 'netbsd'
  demi_src += files('wscons.c')
  subdir('netbsd')
elif host_machine.system() == 'linux'
  demi_src += files('evdev.c')
  subdir('linux')
else
  error('Unsupported platform')
endif
