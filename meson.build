project('CSCoin-Miner', 'c', 'vala')

add_project_arguments('-fopenmp', language: 'c')

posix = meson.get_compiler('vala').find_library('posix')
glib = dependency('glib-2.0')
gobject = dependency('gobject-2.0')
gomp = meson.get_compiler('c').find_library('gomp')

executable('cscoin-miner', 'cscoin-solver.c', 'cscoin-miner.vala', 'contrib/mt19937-64/mt19937-64.c',
           dependencies: [posix, glib, gobject, gomp])

