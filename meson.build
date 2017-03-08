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

solver_lib = library('cscoin-solver', 'cscoin-solver.c', 'cscoin-mt64.c',
                     dependencies: [glib, gio, gomp, openssl])

executable('cscoin-miner', 'cscoin-miner.vala',
           dependencies: [posix, glib, gobject, soup, json_glib, openssl],
           link_with: solver_lib)

subdir('contrib/mt19937-64')
subdir('benchmarks')
subdir('tests')
