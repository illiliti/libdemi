fs = import('fs')
system = target_machine.system()

if fs.is_dir(system)
  subdir(system)
else
  error('Unsupported platform')
endif
