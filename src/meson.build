if get_option('udev')
  subdir('udev')
elif host_machine.system() == 'netbsd'
  subdir('netbsd')
elif host_machine.system() == 'linux'
  subdir('linux')
elif host_machine.system() == 'freebsd'
  subdir('freebsd')
else
  error('Unsupported platform')
endif
