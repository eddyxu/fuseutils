AUTOMAKE_OPTIONS = foreign
AM_CFLAGS = $(fuse_CFLAGS) -DFUSE_USE_VERSION=26 -Wall 
AM_LDFLAGS = $(fuse_LIBS)

bin_PROGRAMS = wrapperfs
wrapperfs_SOURCE = wrapperfs.c
