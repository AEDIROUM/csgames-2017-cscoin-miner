# $Id$
#
# Copyright (C) 2009 Alexios Chouchoulas
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  

COMMON_CFLAGS=-Wall

# Dynamic library

lib_LTLIBRARIES = libastar.la
libastar_ladir = @prefix@/include/libastar
libastar_la_HEADERS = astar.h astar_heap.h astar_config.h
libastar_la_SOURCES = $(libastar_la_HEADERS) astar_heap.c astar.c
libastar_la_CFLAGS = $(COMMON_CFLAGS)
libastar_la_LDFLAGS = -version-info $(LIBVERSION)

# Static library

inst_LIBRARIES = libastar.a
instdir = @prefix@/lib
libastar_adir = $(libastar_ladir)
libastar_a_HEADERS = $(libastar_la_HEADERS)
libastar_a_SOURCES = $(libastar_la_SOURCES)
libastar_a_CFLAGS = $(libastar_la_CFLAGS)

# pkg-config stuff, note that libpng.pc is always required in order
# to get the correct library
pkgconfigdir = @pkgconfigdir@
pkgconfig_DATA = libastar.pc

# Test programs

TESTS = test_heap test_astar debug_heap debug_astar prof_heap prof_astar example

noinst_PROGRAMS=$(TESTS)

test_heap_SOURCES = astar_heap.c astar_heap.h
test_heap_CFLAGS = -DTEST_HEAP

prof_heap_SOURCES = astar_heap.c astar_heap.h
prof_heap_CFLAGS = -DTEST_HEAP -pg -DNUM_INS=1000000

debug_heap_SOURCES = $(test_heap_SOURCES)
debug_heap_CFLAGS = $(test_heap_CFLAGS) -O9 -DHEAP_DEBUG

test_astar_SOURCES = astar_config.h astar.c astar.h astar_heap.c astar_heap.h
test_astar_CFLAGS = -DTEST_ASTAR -pg

prof_astar_SOURCES = $(test_astar_SOURCES)
prof_astar_CFLAGS = $(test_astar_CFLAGS) -pg -DNUM_REPS=5000

debug_astar_SOURCES = $(test_astar_SOURCES)
#debug_astar_CFLAGS = $(test_astar_CFLAGS) -DASTAR_DEBUG
debug_astar_CFLAGS = $(test_astar_CFLAGS) -O9 -DASTAR_DEBUG -DHEAP_DEBUG

example_SOURCES = example.c
example_CFLAGS = -DASTAR_BUILD
example_LDFLAGS = libastar.a

# End of file.