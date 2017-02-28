project('CSCoin-Miner', 'c', 'vala')

posix = meson.get_compiler('vala').find_library('posix')
glib = dependency('glib-2.0')
gobject = dependency('gobject-2.0')

executable('cscoin-miner', 'cscoin-solver.c', 'cscoin-miner.vala', 'contrib/mt19937-64/mt19937-64.c',
           dependencies: [posix, glib, gobject])

