/*
 * Copyright 2010-2013 (c) Lei Xu <eddyxu@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \brief Wrapper File System.
 *
 * It works as a pesudo-file-system running on top of existing file system
 * (Ext3/4, Btrfs, HFS+ and etc).
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <fuse_opt.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>
#include <cstdio>
#include <string>
#include "./config.h"

using std::string;

#define CALL_RETURN(x) return (x) == -1 ? -errno : 0;

/** command line options */
struct options {
  char *basedir;
} options;

string wrapperfs_abspath(const string &path) {
  return string(options.basedir) + path;
}

int wrapperfs_getattr(const char *path, struct stat *stbuf) {
  string abspath = wrapperfs_abspath(path);
  CALL_RETURN(stat(abspath.c_str(), stbuf));
}

int wrapperfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
               off_t offset, struct fuse_file_info *fi) {
  (void) offset;
  (void) fi;

  int res = 0;
  string abspath = wrapperfs_abspath(path);

  DIR *dirp = opendir(abspath.c_str());
  if (dirp == NULL) {
    return -errno;
  }

  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);

  struct dirent *dp;
  while ((dp = readdir(dirp)) != NULL) {
    filler(buf, dp->d_name, NULL, 0);
  }
  closedir(dirp);
  return res;
}

int wrapperfs_open(const char *path, struct fuse_file_info *fi) {
  string abspath = wrapperfs_abspath(path);
  int fd = open(abspath.c_str(), fi->flags);
  if (fd == -1) {
    return -errno;
  };
  fi->fh = fd;
  return 0;
}

int wrapperfs_create(const char *path, mode_t mode,
                     struct fuse_file_info *fi) {
  string abspath = wrapperfs_abspath(path);
  int fd = creat(abspath.c_str(), mode);
  if (fd == -1) {
    return -errno;
  };
  fi->fh = fd;
  return 0;
}

int wrapperfs_release(const char *path , struct fuse_file_info *fi) {
  (void) path;
  CALL_RETURN(close(fi->fh));
}

int wrapperfs_read(const char *path, char *buf, size_t size, off_t offset,
                   struct fuse_file_info *fi) {
  (void) path;
  ssize_t nread = pread(fi->fh, buf, size, offset);
  if (nread == -1) {
    return -errno;
  }
  return nread;
}

int wrapperfs_write(const char *path, const char *buf, size_t size,
                    off_t offset, struct fuse_file_info *fi) {
  (void) path;
  ssize_t nwrite = pwrite(fi->fh, buf, size, offset);
  if (nwrite == -1) {
    return -errno;
  }
  return nwrite;
}

int wrapperfs_access(const char *path, int flag) {
  string abspath = wrapperfs_abspath(path);
  CALL_RETURN(access(abspath.c_str(), flag));
}

int wrapperfs_chmod(const char *path, mode_t mode) {
  string abspath = wrapperfs_abspath(path);
  CALL_RETURN(chmod(abspath.c_str(), mode));
}

int wrapperfs_chown(const char *path, uid_t owner, gid_t group) {
  string abspath = wrapperfs_abspath(path);
  CALL_RETURN(chown(abspath.c_str(), owner, group));
}

int wrapperfs_utimens(const char *path, const struct timespec tv[2]) {
  string abspath = wrapperfs_abspath(path);
  struct timeval times[2];
  times[0].tv_sec = tv[0].tv_sec;
  times[0].tv_usec = tv[0].tv_nsec / 1000;
  times[1].tv_sec = tv[1].tv_sec;
  times[1].tv_usec = tv[1].tv_nsec / 1000;
  CALL_RETURN(utimes(abspath.c_str(), times));
}

int wrapperfs_unlink(const char *path) {
  string abspath = wrapperfs_abspath(path);
  CALL_RETURN(unlink(abspath.c_str()));
}

int wrapperfs_rename(const char *oldpath, const char *newpath) {
  string abs_oldpath = wrapperfs_abspath(oldpath);
  string abs_newpath = wrapperfs_abspath(newpath);
  CALL_RETURN(rename(abs_oldpath.c_str(), abs_newpath.c_str()));
}

int wrapperfs_link(const char *path1, const char *path2) {
  string abs_path1 = wrapperfs_abspath(path1);
  string abs_path2 = wrapperfs_abspath(path2);
  CALL_RETURN(link(abs_path1.c_str(), abs_path2.c_str()));
}

int wrapperfs_symlink(const char *path1, const char *path2) {
  string abs_path1;
  /*
   * FUSE pass out-of-partition source directory path as absolute path,
   * and pass in-partition source directory as related path
   */
  if (path1[0] == '/') {
    abs_path1 = path1;
  } else {
    abs_path1 = wrapperfs_abspath(path1);
  }
  string abs_path2 = wrapperfs_abspath(path2);
  CALL_RETURN(symlink(abs_path1.c_str(), abs_path2.c_str()));
}

int wrapperfs_truncate(const char *path, off_t length) {
  string abspath = wrapperfs_abspath(path);
  CALL_RETURN(truncate(abspath.c_str(), length));
}

int wrapperfs_mkdir(const char *path, mode_t mode) {
  string abspath = wrapperfs_abspath(path);
  CALL_RETURN(mkdir(abspath.c_str(), mode));
}

int wrapperfs_rmdir(const char *path) {
  string abspath = wrapperfs_abspath(path);
  CALL_RETURN(rmdir(abspath.c_str()));
}

#define WRAPPERFS_OPT_KEY(t, p, v) { t, offsetof(struct options, p), v }
enum {
  KEY_VERSION,
  KEY_HELP,
  KEY_DEBUG,
};

struct fuse_opt wrapperfs_opts[] = {
  WRAPPERFS_OPT_KEY("--basedir %s", basedir, 0),
  WRAPPERFS_OPT_KEY("-b %s", basedir, 0),

  FUSE_OPT_KEY("--version", KEY_VERSION),
  FUSE_OPT_KEY("-h", KEY_HELP),
  FUSE_OPT_KEY("--help", KEY_HELP),
  FUSE_OPT_KEY("--debug", KEY_DEBUG),
  FUSE_OPT_END
};

int wrapperfs_opt_proc(void *data, const char *arg, int key,
                       struct fuse_args *outargs) {
  fuse_operations opers;
  int res = 1;
  switch (key) {
  case KEY_HELP:
    fprintf(stderr, "Usage: %s mountpoint [options]\n"
        "\n"
        "General options:\n"
        "  -o opt,[opt...]\tmount options\n"
        "  -h, --help\t\tdisplay this help\n"
        "  --version\t\tshow version information\n"
        "  -d, --debug\t\trun in debug mode\n"
        "\n"
        "Mount options:\n"
        "  -b, --basedir DIR\tmount target directory\n"
        "\n"
        , outargs->argv[0]);
    fuse_opt_add_arg(outargs, "-ho");
    fuse_main(outargs->argc, outargs->argv, &opers, NULL);
    exit(1);
  case KEY_VERSION:
    // TODO(eddyxu): move the version information to config.h
    fprintf(stderr, "Wrapper version: %s\n", PACKAGE_VERSION);
    fuse_opt_add_arg(outargs, "--version");
    fuse_main(outargs->argc, outargs->argv, &opers, NULL);
    exit(1);
  case KEY_DEBUG:
    fuse_opt_add_arg(outargs, "-d");
    res = 0;  // this arg is to be discarded
    break;
  }
  return res;
}


int main(int argc, char *argv[]) {
  int ret = 0;

  fuse_operations opers;
  opers.access = wrapperfs_access;
  opers.chmod = wrapperfs_chmod;
  opers.chown = wrapperfs_chown;
  opers.create = wrapperfs_create;
  opers.getattr = wrapperfs_getattr;
  opers.link = wrapperfs_link;
  opers.mkdir = wrapperfs_mkdir;
  opers.open = wrapperfs_open;
  opers.read = wrapperfs_read;
  opers.readdir = wrapperfs_readdir;
  opers.release = wrapperfs_release;
  opers.rename = wrapperfs_rename;
  opers.rmdir = wrapperfs_rmdir;
  opers.symlink = wrapperfs_symlink;
  opers.truncate = wrapperfs_truncate;
  opers.unlink = wrapperfs_unlink;
  opers.utimens = wrapperfs_utimens;
  opers.write = wrapperfs_write;

  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
  if (fuse_opt_parse(&args, &options, wrapperfs_opts,
                     wrapperfs_opt_proc) == -1) {
    ret = -1;
    goto exit_handler;
  }

  if (options.basedir == NULL) {
    fprintf(stderr, "Must provide base directory.\n");
    ret = 1;
    goto exit_handler;
  }

  if (access(options.basedir, F_OK) == -1) {
    perror("Target directory");
    ret = 1;
    goto exit_handler;
  }

  fprintf(stderr, "Mount %s to %s.\n", args.argv[0], options.basedir);
  ret = fuse_main(args.argc, args.argv, &opers, NULL);

  if (ret)
    fprintf(stderr, "\n");

exit_handler:  // NOLINT
  fuse_opt_free_args(&args);
  return ret;
}
