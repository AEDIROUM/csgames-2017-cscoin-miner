project('CSCoin-Miner', 'c', 'vala')

add_project_arguments('-fopenmp', language: 'c')
add_project_arguments('--vapidir=' + meson.current_source_dir(), language: 'vala')

posix = meson.get_compiler('vala').find_library('posix')
glib = dependency('glib-2.0')
gobject = dependency('gobject-2.0')
gomp = meson.get_compiler('c').find_library('gomp')
soup = dependency('libsoup-2.4')
json_glib = dependency('json-glib-1.0')
openssl = dependency('openssl')

executable('cscoin-miner', 'cscoin-solver.c', 'cscoin-miner.vala', 'contrib/mt19937-64/mt19937-64.c',
           dependencies: [posix, glib, gobject, gomp, soup, json_glib, openssl])
