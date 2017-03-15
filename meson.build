project('CSCoin-Miner', 'c', 'vala')

add_project_arguments('-fopenmp', language: 'c')
add_project_arguments('--vapidir=' + meson.current_source_dir(), language: 'vala')

posix = meson.get_compiler('vala').find_library('posix')
glib = dependency('glib-2.0')
gobject = dependency('gobject-2.0')
gio = dependency('gio-2.0')
gomp = meson.get_compiler('c').find_library('gomp')
soup = dependency('libsoup-2.4')
json_glib = dependency('json-glib-1.0')
openssl = dependency('openssl')

solver_lib = library('cscoin-solver', 'cscoin-solver.c', 'cscoin-mt64.c', 'cscoin-challenge-type.c', 'cscoin-challenge-parameters.c',
                     dependencies: [glib, gio, gomp, openssl])
solver = declare_dependency(link_with: solver_lib, include_directories: include_directories('.'))
solver_vapi = meson.get_compiler('vala').find_library('cscoin-solver', dirs: meson.current_source_dir())

executable('cscoin-miner', 'cscoin-miner.vala', 'cscoin-challenge.vala',
           dependencies: [posix, glib, gobject, soup, json_glib, openssl, solver, solver_vapi])

subdir('contrib/mt19937-64')
subdir('benchmarks')
subdir('tests')
