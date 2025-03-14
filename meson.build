project('tftp', ['cpp'],
	version: '1.3.0',
	default_options: 'cpp_std=c++17',
	license: 'Unlicense'
)

tftp_dep = declare_dependency(
	include_directories: include_directories('./tftp')
).as_system('system')

install_headers('tftp/packets.hpp' 'tftp/parser.hpp', 'tftp/tftp.hpp', subdir: 'tftp')
