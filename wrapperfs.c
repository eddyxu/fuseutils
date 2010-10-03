/**
 * \brief Wrapper File System
 *
 * It works as a pesudo-file-system on top of existing file system (Ext3/4,
 * Btrfs, HFS+ and etc).
 *
 * \author Lei Xu <lxu@cse.unl.edu>
 * Copyright 2010 (c) University of Nebraska-Lincoln
 */

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <stddef.h>
#include <stdarg.h>
#include <utime.h>
#include <fcntl.h>
#include <assert.h>
#include <fuse.h>
#include <fuse_opt.h>

#include "config.h"

#define BUFSIZE 1024

/** command line options */
struct options {
	char *basedir;
} options;


enum {
	LOG_INFO,
	LOG_ERROR,
	LOG_WARNING,
	LOG_DEBUG,
	LOG_UNKNOWN,
};


static void wrapperfs_debug(int log_level, char *format, ...) 
{
	static const char * LOG_TAGS[] = { "INFO", "ERROR", "WARNING", "DEBUG" };
	assert (log_level < LOG_UNKNOWN);
	fprintf(stderr, "[%s] ", LOG_TAGS[log_level]);
	va_list args;
	va_start (args, format);
	vfprintf(stderr, format, args);
	va_end (args);
	fprintf(stderr, "\n");
	fflush(stderr);
}


static void wrapperfs_abspath(const char *path, char *buffer, size_t bufsize) 
{
	assert(buffer);
	memset(buffer, 0, bufsize);
	snprintf(buffer, bufsize, "%s%s", options.basedir, path);
	buffer[BUFSIZE-1] = '\0';
}


static int wrapperfs_getattr(const char *path, struct stat *stbuf) 
{
	char abspath[BUFSIZE];
	wrapperfs_abspath(path, abspath, BUFSIZE);
	memset(stbuf, 0, sizeof(struct stat));
	return stat(abspath, stbuf) == -1 ? -errno: 0;
}


static int wrapperfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi) 
{
	(void) offset;
	(void) fi;

	int res = 0;
	char abspath[BUFSIZE];
	wrapperfs_abspath(path, abspath, BUFSIZE);

	DIR *dirp = opendir(abspath);
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


static int wrapperfs_open(const char *path, struct fuse_file_info *fi) 
{
	char abspath[BUFSIZE];
	wrapperfs_abspath(path, abspath, BUFSIZE);
	int fd = open(abspath, fi->flags);
	if (fd == -1) {
		return -errno;
	};
	fi->fh = fd;
	return 0;
}


static int wrapperfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	char abspath[BUFSIZE];
	wrapperfs_abspath(path, abspath, BUFSIZE);
	int fd = creat(abspath, mode);
	if (fd == -1) {
		return -errno;
	};
	fi->fh = fd;
	return 0;
}


static int wrapperfs_release(const char *path , struct fuse_file_info *fi) {
	(void) path;
	return close(fi->fh) == -1 ? -errno : 0;
}


static int wrapperfs_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi)
{
	(void) path;
	return pread(fi->fh, buf, size, offset);
}


static int wrapperfs_write(const char *path, const char *buf, size_t size, 
						off_t offset, struct fuse_file_info *fi) 
{
	(void) path;
	return pwrite(fi->fh, buf, size, offset);
}


static int wrapperfs_access(const char *path, int flag) {
	char abspath[BUFSIZE];
	wrapperfs_abspath(path, abspath, BUFSIZE);
	return access(abspath, flag) == -1 ? -errno : 0;
}


static int wrapperfs_chmod(const char *path, mode_t mode) {
	char abspath[BUFSIZE];
	wrapperfs_abspath(path, abspath, BUFSIZE);
	return chmod(abspath, mode) == -1 ? -errno : 0;
}


static int wrapperfs_chown(const char *path, uid_t owner, gid_t group) 
{
	char abspath[BUFSIZE];
	wrapperfs_abspath(path, abspath, BUFSIZE);
	return chown(abspath, owner, group) == -1 ? -errno : 0;
}


static int wrapperfs_utimens(const char *path, const struct timespec tv[2]) 
{
	char abspath[BUFSIZE];
	struct timeval times[2];
	times[0].tv_sec = tv[0].tv_sec;
	times[0].tv_usec = tv[0].tv_nsec / 1000;
	times[1].tv_sec = tv[1].tv_sec;
	times[1].tv_usec = tv[1].tv_nsec / 1000;

	wrapperfs_abspath(path, abspath, BUFSIZE);

	return utimes(abspath, times) == -1 ? -errno : 0;
}


static int wrapperfs_unlink(const char *path) 
{
	char abspath[BUFSIZE];
	wrapperfs_abspath(path, abspath, BUFSIZE);
	return unlink(abspath) == -1 ? -errno : 0;
}


static int wrapperfs_rename(const char *oldpath, const char *newpath) {
	char abs_oldpath[BUFSIZE];
	char abs_newpath[BUFSIZE];
	wrapperfs_abspath(oldpath, abs_oldpath, BUFSIZE);
	wrapperfs_abspath(newpath, abs_newpath, BUFSIZE);
	return rename(abs_oldpath, abs_newpath) == -1 ? -errno : 0;
}


static int wrapperfs_link(const char *path1, const char *path2) {
	char abs_path1[BUFSIZE];
	char abs_path2[BUFSIZE];
	wrapperfs_abspath(path1, abs_path1, BUFSIZE);
	wrapperfs_abspath(path2, abs_path2, BUFSIZE);
	return link(abs_path1, abs_path2) == -1 ? -errno : 0;
}


static int wrapperfs_symlink(const char *path1, const char *path2) {
	char abs_path1[BUFSIZE];
	char abs_path2[BUFSIZE];
	wrapperfs_abspath(path1, abs_path1, BUFSIZE);
	wrapperfs_abspath(path2, abs_path2, BUFSIZE);
	return symlink(abs_path1, abs_path2) == -1 ? -errno : 0;
}


static int wrapperfs_truncate(const char *path, off_t length) 
{
	char abspath[BUFSIZE];
	wrapperfs_abspath(path, abspath, BUFSIZE);
	return truncate(abspath, length) == -1 ? -errno : 0;
}


static int wrapperfs_mkdir(const char *path, mode_t mode) 
{
	char abspath[BUFSIZE];
	wrapperfs_abspath(path, abspath, BUFSIZE);
	return mkdir(abspath, mode) == -1 ? -errno : 0;
}


static int wrapperfs_rmdir(const char *path) {
	char abspath[BUFSIZE];
	wrapperfs_abspath(path, abspath, BUFSIZE);
	return rmdir(abspath) == -1 ? -errno : 0;
}


static struct fuse_operations wrapperfs_operations = {
	.access = wrapperfs_access,
	.chmod = wrapperfs_chmod,
	.chown = wrapperfs_chown,
	.create = wrapperfs_create,
	.getattr = wrapperfs_getattr,
	.link = wrapperfs_link,
	.mkdir = wrapperfs_mkdir,
	.open = wrapperfs_open,
	.read = wrapperfs_read,
	.readdir = wrapperfs_readdir,
	.release = wrapperfs_release,
	.rename = wrapperfs_rename,
	.rmdir = wrapperfs_rmdir,
	.symlink = wrapperfs_symlink,
	.truncate = wrapperfs_truncate,
	.unlink = wrapperfs_unlink,
	.utimens = wrapperfs_utimens,
	.write = wrapperfs_write,
};


#define WRAPPERFS_OPT_KEY(t, p, v) { t, offsetof(struct options, p), v }
enum {
	KEY_VERSION,
	KEY_HELP,
	KEY_DEBUG,
};


static struct fuse_opt wrapperfs_opts[] = {
	WRAPPERFS_OPT_KEY("--basedir %s", basedir, 0),
	WRAPPERFS_OPT_KEY("-b %s", basedir, 0),

	FUSE_OPT_KEY("--version", KEY_VERSION),
	FUSE_OPT_KEY("-h", KEY_HELP),
	FUSE_OPT_KEY("--help", KEY_HELP),
	FUSE_OPT_KEY("--debug", KEY_DEBUG),
	FUSE_OPT_END
};


static int wrapperfs_opt_proc(void *data, const char *arg, int key, struct fuse_args *outargs) 
{
	int res = 1;
	switch (key) {
	case KEY_HELP:
		fprintf(stderr, "Usage: %s mountpoint [options]\n"
				"\n"
				"General options:\n"
				"    -o opt,[opt...]\tmount options\n"
				"    -h, --help\t\tdisplay this help\n"
				"    --version\t\tshow version information\n"
				"    -d, --debug\t\trun in debug mode\n"
				"\n"
				"Mount options:\n"
				"    -b, --basedir DIR\tmount target directory\n"
				"\n"
				, outargs->argv[0]);
		fuse_opt_add_arg(outargs, "-ho");
		fuse_main(outargs->argc, outargs->argv, &wrapperfs_operations, NULL);
		exit(1);
	case KEY_VERSION:
		// TODO: move the version information to config.h
		fprintf(stderr, "Wrapper version: %s\n", PACKAGE_VERSION);
		fuse_opt_add_arg(outargs, "--version");
		fuse_main(outargs->argc, outargs->argv, &wrapperfs_operations, NULL);
		exit(1);
	case KEY_DEBUG:
		fuse_opt_add_arg(outargs, "-d");
		res = 0;  // this arg is to be discarded
		break;
	}
	return 1;
}


int main(int argc, char *argv[]) {
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	memset(&options, 0, sizeof(struct options));

	if (fuse_opt_parse(&args, &options, wrapperfs_opts, wrapperfs_opt_proc) == -1) {
		ret = -1;
		goto exit_handler;
	}

	if (options.basedir == NULL) {
		wrapperfs_debug(LOG_ERROR, "You have to point out targeted directory");
		ret = 1;
		goto exit_handler;
	} 

	if (access(options.basedir, F_OK) == -1) {
		perror("Targeted directory");
		ret = 1;
		goto exit_handler;
	}

	wrapperfs_debug(LOG_INFO, "Mount %s to %s", args.argv[0], options.basedir);
	ret = fuse_main(args.argc, args.argv, &wrapperfs_operations, NULL);

	if (ret) 
		printf("\n");

exit_handler:
	fuse_opt_free_args(&args);

	return ret;
}