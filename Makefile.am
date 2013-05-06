AUTOMAKE_OPTIONS = foreign
AM_CXXFLAGS = $(fuse_CFLAGS) -DFUSE_USE_VERSION=29 -Wall -DFILE_OFFSET_BITS=64 -pedantic -Wparentheses
LDADD = $(fuse_LIBS)

bin_PROGRAMS = wrapperfs
wrapperfs_SOURCES = wrapperfs.cpp
