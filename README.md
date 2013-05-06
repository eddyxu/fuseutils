 fuseutils -- Fuse development utilities
==========================================

[![Build Status](https://travis-ci.org/eddyxu/fuseutils.png?branch=master)](https://travis-ci.org/eddyxu/fuseutils)

## Introduction
 This package collects some useful data structures for fuse development

## Components:

 * wrapperfs.cpp: a simple fuse file system that run on top of existing file
   systems. It can be used as a start point to evaluate file system
   designs.

## Development

Dependencies:

 * FUSE >= 2.8
 * g++ >= 4.7.2
 
Install dependencies on Ubuntu

```sh
$ sudo apt-get install libfuse-dev g++
```

Build the source code:

```
$ ./bootstrap
$ ./configure
$ make
```

## Author:
   Lei Xu <eddyxu@gmail.com>
