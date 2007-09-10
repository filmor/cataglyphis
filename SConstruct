# -*- encoding:utf8 -*-

import glob
import re
from os.path import join

name = "cataglyphis"
sources = glob.glob('src/*.cpp')
prefix = "/Users/bs/Gentoo/usr"
libdir = [ '.', join(prefix, "lib") ]
include = [ join(i, "include") for i in ["/usr", prefix, "asio", "libtorrent"] ]
libs = "boost_thread boost_filesystem torrent-0.13".split()
cxxflags = "-Wall -g3 -fomit-frame-pointer -march=k8" # -D_GLIBCXX_CONCEPT_CHECKS"

ldflags = "".split ()

env = Environment(CXXFLAGS=cxxflags, CPPPATH=include, LIBPATH=libdir, LIBS=libs, LINKFLAGS = ldflags)

programs = { "src/main.cpp" : "cataglyphis" }
objects = [ env.Object(i) for i in sources if not i in programs ]

# env.Program(name, objects + ['src/main.cpp'])
for i in programs:
    env.Program(programs[i], [i] + objects)
