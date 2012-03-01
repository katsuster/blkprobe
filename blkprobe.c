#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/fs.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static char *image_file = NULL;
static int image_fd = -1;
static char *log_file = NULL;
static int log_fd = -1;
static const char *through_path = "/image";

struct access_log_header {
	uint64_t capacity;
};

struct access_history {
	uint32_t op;
	uint64_t address;
};

static uint64_t bswap64(uint64_t n)
{
	int k = 56;
	
	if (k &  1) n = (n & 0x5555555555555555ULL) <<  1 | 
	                (n & 0xaaaaaaaaaaaaaaaaULL) >>  1;
	if (k &  2) n = (n & 0x3333333333333333ULL) <<  2 | 
	                (n & 0xccccccccccccccccULL) >>  2;
	if (k &  4) n = (n & 0x0f0f0f0f0f0f0f0fULL) <<  4 | 
	                (n & 0xf0f0f0f0f0f0f0f0ULL) >>  4;
	if (k &  8) n = (n & 0x00ff00ff00ff00ffULL) <<  8 | 
	                (n & 0xff00ff00ff00ff00ULL) >>  8;
	if (k & 16) n = (n & 0x0000ffff0000ffffULL) << 16 | 
	                (n & 0xffff0000ffff0000ULL) >> 16;
	if (k & 32) n = (n & 0x00000000ffffffffULL) << 32 | 
	                (n & 0xffffffff00000000ULL) >> 32;
	
	return n;
}

static uint32_t bswap32(uint32_t n)
{
	int k = 24;
	
	if (k &  1) n = (n & 0x55555555UL) <<  1 | 
	                (n & 0xaaaaaaaaUL) >>  1;
	if (k &  2) n = (n & 0x33333333UL) <<  2 | 
	                (n & 0xccccccccUL) >>  2;
	if (k &  4) n = (n & 0x0f0f0f0fUL) <<  4 | 
	                (n & 0xf0f0f0f0UL) >>  4;
	if (k &  8) n = (n & 0x00ff00ffUL) <<  8 | 
	                (n & 0xff00ff00UL) >>  8;
	if (k & 16) n = (n & 0x0000ffffUL) << 16 | 
	                (n & 0xffff0000UL) >> 16;
	
	return n;
}

static int hello_getattr(const char *path, struct stat *stbuf)
{
	int result = 0;
	long sectors;

	printf("[!] %s\n", __func__);

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
		stbuf->st_uid = geteuid();
		stbuf->st_gid = getgid();
	} else if (strcmp(path, through_path) == 0) {
		result = fstat(image_fd, stbuf);
		if (result == -1) {
			result = errno;
			goto errout;
		}

		printf("before: dev:%d, size:%lld, blksize:%d, blocks:%lld\n", 
			(int)stbuf->st_dev, stbuf->st_size, 
			(int)stbuf->st_blksize, stbuf->st_blocks);

		if (S_ISBLK(stbuf->st_mode)) {
			ioctl(image_fd, BLKGETSIZE, &sectors);
			if (result < 0) {
				result = errno;
				goto errout;
			}
			printf("blkdev: sectors:%ld\n", sectors);
			//通常ファイルを偽装しているので PAGE_SIZE 単位で
			//read/write 要求がくる。
			//PAGE_SIZE の倍数に揃える。今は 4KB 固定。
			sectors &= ~0x7;

			stbuf->st_mode &= ~S_IFMT;
			stbuf->st_mode |= S_IFREG;
			stbuf->st_size = (off_t)sectors * 512;
			stbuf->st_blksize = 512;
			stbuf->st_blocks = sectors;
		}
		stbuf->st_nlink = 1;

		printf("after : dev:%d, size:%lld, blksize:%d, blocks:%lld\n", 
			(int)stbuf->st_dev, stbuf->st_size, 
			(int)stbuf->st_blksize, stbuf->st_blocks);
	} else {
		result = -ENOENT;
	}

errout:
	return result;
}

static int hello_readlink(const char *path, char *buf, size_t size)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_mknod(const char *path, mode_t mode, dev_t device)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_mkdir(const char *path, mode_t mode)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_unlink(const char *path)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_rmdir(const char *path)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_symlink(const char *linkname, const char *path)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_rename(const char *oldpath, const char *newpath)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_link(const char *oldpath, const char *newpath)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_chmod(const char *path, mode_t mode)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_chown(const char *path, uid_t uid, gid_t gid)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_truncate(const char *path, off_t offset)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	if (strcmp(path, through_path) != 0) {
		return -ENOENT;
	}

	if ((fi->flags & 3) != O_RDONLY) {
	//	return -EACCES;
	}

	return result;
}

static int hello_read(const char *path, char *buf, size_t size,
		      off_t offset, struct fuse_file_info *fi)
{
	int result = 0;
	ssize_t nread, total;
	struct access_history h;
	off_t now;

	//printf("[!] %s %08llx-%08llx\n", __func__, 
	//	(uint64_t)offset, (uint64_t)offset + size);

	if (strcmp(path, through_path) != 0) {
		return -ENOENT;
	}
	
	pthread_mutex_lock(&lock);
	
	now = lseek(image_fd, offset, SEEK_SET);
	if (now != offset) {
		result = errno;
		goto errout;
	}
	
	for (total = 0, nread = 0; total < size; total += nread) {
		nread = read(image_fd, buf + total, size - total);
		if (nread == 0) {
			//eof
			fprintf(stderr, 
				"%s: reach EOF, offset:0x%llx, "
				"size:%lld, total:%lld.\n", 
				__func__, offset, 
				(uint64_t)size, (uint64_t)total);
			result = errno;
			goto errout;
		} else if (nread == -1) {
			if (errno == EINTR) {
				//interrupted, try again
				nread = 0;
				continue;
			} else {
				result = errno;
				goto errout;
			}
		}
	}
	
	pthread_mutex_unlock(&lock);
	
	//logging
	memset(&h, 0, sizeof(h));
	h.op = bswap32(1);
	h.address = bswap64(offset);
	write(log_fd, &h, sizeof(h));
	
	return nread;
errout:
	perror("hello_read");
	
	return result;
}

static int hello_write(const char *path, const char *buf, size_t size,
		       off_t offset, struct fuse_file_info *fi)
{
	int result = 0;
	ssize_t nwrite, total;
	struct access_history h;
	off_t now;

	//printf("[!] %s %08llx-%08llx\n", __func__, 
	//	(uint64_t)offset, (uint64_t)offset + size);

	if (strcmp(path, through_path) != 0) {
		return -ENOENT;
	}
	
	pthread_mutex_lock(&lock);
	
	now = lseek(image_fd, offset, SEEK_SET);
	if (now != offset) {
		result = errno;
		goto errout;
	}
	
	for (total = 0, nwrite = 0; total < size; total += nwrite) {
		nwrite = write(image_fd, buf + total, size - total);
		if (nwrite == 0) {
			//eof
			fprintf(stderr, 
				"%s: reach EOF, offset:0x%llx, "
				"size:%lld, total:%lld.\n", 
				__func__, offset, 
				(uint64_t)size, (uint64_t)total);
			result = errno;
			goto errout;
		} else if (nwrite == -1) {
			if (errno == EINTR) {
				//interrupted, try again
				nwrite = 0;
				continue;
			} else {
				result = errno;
				goto errout;
			}
		}
	}

	pthread_mutex_unlock(&lock);
	
	//logging
	memset(&h, 0, sizeof(h));
	h.op = bswap32(2);
	h.address = bswap64(offset);
	write(log_fd, &h, sizeof(h));
	
	return nwrite;
errout:
	perror("hello_write");
	
	return result;
}

static int hello_statfs(const char *path, struct statvfs *buf)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_flush(const char *path, struct fuse_file_info *fi)
{
	int result = 0;

	printf("[!] %s\n", __func__);
	
	result = fdatasync(image_fd);
	if (result == -1) {
		result = errno;
		goto errout;
	}

	return result;
errout:
	perror("hello_flush");
	
	return result;
}

static int hello_release(const char *path, struct fuse_file_info *fi)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_fsync(const char *path, int datasync,
		       struct fuse_file_info *fi)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	result = fsync(image_fd);
	if (result == -1) {
		result = errno;
		goto errout;
	}

	return result;
errout:
	perror("hello_fsync");
	
	return result;
}

static int hello_setxattr(const char *path, const char *name,
			  const char *value, size_t size, int flags)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_getxattr(const char *path, const char *name, char *value,
			  size_t size)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_listxattr(const char *path, char *list, size_t size)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_removexattr(const char *path, const char *name)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_opendir(const char *path, struct fuse_file_info *fi)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_readdir(const char *path, void *buf,
			 fuse_fill_dir_t filler, off_t off,
			 struct fuse_file_info *fi)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	if (strcmp(path, "/") != 0) {
		return -ENOENT;
	}

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, through_path + 1, NULL, 0);

	return result;
}

static int hello_fsyncdir(const char *path, int datasync,
			  struct fuse_file_info *fi)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_releasedir(const char *path, struct fuse_file_info *fi)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static void *hello_init(struct fuse_conn_info *conn)
{
	printf("[!] %s\n", __func__);
	
	return NULL;
}

static void hello_destroy(void *ptr)
{
	printf("[!] %s\n", __func__);
	
	if (image_fd != -1) {
		close(image_fd);
		image_file = NULL;
		image_fd = -1;
	}
	if (log_fd != -1) {
		close(log_fd);
		log_file = NULL;
		log_fd = -1;
	}
}

static int hello_access(const char *path, int mask)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_create(const char *path, mode_t mode,
			struct fuse_file_info *fi)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_ftruncate(const char *path,
			   off_t size, struct fuse_file_info *fi)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_fgetattr(const char *path, struct stat *buf,
			  struct fuse_file_info *fi)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_lock(const char *path, struct fuse_file_info *fi, int cmd,
		      struct flock *lock)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_utimens(const char *path, const struct timespec tv[2])
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_bmap(const char *path, size_t blocksize, uint64_t * idx)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_ioctl(const char *path, int cmd,
		       void *arg, struct fuse_file_info *fi,
		       unsigned int flags, void *data)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static int hello_poll(const char *path,
		      struct fuse_file_info *fi,
		      struct fuse_pollhandle *ph, unsigned *reventsp)
{
	int result = 0;

	printf("[!] %s\n", __func__);

	return result;
}

static struct fuse_operations hello_oper = {
	.getattr = hello_getattr,
	.readlink = hello_readlink,
	//.getdir      = NULL, //deprecated
	.mknod = hello_mknod,
	.mkdir = hello_mkdir,
	.unlink = hello_unlink,
	.rmdir = hello_rmdir,
	.symlink = hello_symlink,
	.rename = hello_rename,
	.link = hello_link,
	.chmod = hello_chmod,
	.chown = hello_chown,
	.truncate = hello_truncate,
	//.utime       = NULL, //deprecated
	.open = hello_open,
	.read = hello_read,
	.write = hello_write,
	.statfs = hello_statfs,
	.flush = hello_flush,
	.release = hello_release,
	.fsync = hello_fsync,
	.setxattr = hello_setxattr,
	.getxattr = hello_getxattr,
	.listxattr = hello_listxattr,
	.removexattr = hello_removexattr,
	.opendir = hello_opendir,
	.readdir = hello_readdir,
	.releasedir = hello_releasedir,
	.fsyncdir = hello_fsyncdir,
	.init = hello_init,
	.destroy = hello_destroy,
	.access = hello_access,
	.create = hello_create,
	.ftruncate = hello_ftruncate,
	.fgetattr = hello_fgetattr,
	.lock = hello_lock,
	.utimens = hello_utimens,
	.bmap = hello_bmap,
	.flag_nullpath_ok = 0,
	//.flag_reserved = 0, //don't set
	.ioctl = hello_ioctl,
	.poll = hello_poll,
};

int main(int argc, char *argv[])
{
	int fuse_argc;
	char **fuse_argv;
	int i, j;
	struct addrinfo hint;
	struct addrinfo *addrs;
	struct stat st;
	struct access_log_header h;
	int result;
	ssize_t nwrite;
	
	if (argc < 2) {
		fprintf(stderr, 
			"usage:\n"
			"%s imagefile mountpoint [options]\n", 
			argv[0]);
		return 1;
	}
	
	fuse_argc = argc - 2;
	fuse_argv = (char **)calloc(sizeof(char *), fuse_argc);
	for (i = 0, j = 0; i < argc; i++) {
		if (i == 1) {
			image_file = argv[i];
			continue;
		}
		if (i == 2) {
			log_file = argv[i];
			continue;
		}
		fuse_argv[j] = argv[i];
		j++;
	}
	
	image_fd = open(image_file, O_RDWR);
	if (image_fd == -1) {
		fprintf(stderr, "image file '%s' is not found.\n", 
			image_file);
		perror("open");
		return 1;
	}
	
	/*
	log_fd = open(log_file, O_WRONLY);
	if (log_fd == -1) {
		fprintf(stderr, "log file '%s' is not found.\n", 
			log_file);
		perror("open");
		return 1;
	}
	*/
	
	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;
	result = getaddrinfo(log_file, "10001", &hint, &addrs);
	if (result != 0) {
		goto errout;
	}
	
	log_fd = socket(addrs->ai_family, addrs->ai_socktype, 
		addrs->ai_protocol);
	if (log_fd == -1) {
		result = errno;
		goto errout;
	}
	
	result = connect(log_fd, addrs->ai_addr, addrs->ai_addrlen);
	if (result == -1) {
		result = errno;
		goto errout;
	}
	
	freeaddrinfo(addrs);
	
	//logging header
	memset(&st, 0, sizeof(st));
	result = hello_getattr(through_path, &st);
	if (result == -1) {
		result = errno;
		goto errout;
	}
	
	memset(&h, 0, sizeof(h));
	h.capacity = bswap64(st.st_size);
	nwrite = write(log_fd, &h, sizeof(h));
	if (nwrite != sizeof(h)) {
		result = errno;
		goto errout;
	}
	
	return fuse_main(fuse_argc, fuse_argv, &hello_oper, NULL);

errout:
	perror("main");
	return result;
}
