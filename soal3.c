#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/stat.h>

char dirpath[2048];


static int E4_getattr(const char *path, struct stat *stbuf)
{
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);
	res = lstat(fpath, stbuf);
	if (res == -1)
		return -errno;
	
	return 0;
}

static int E4_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
 	char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;
	while ((de = readdir(dp)) != NULL) {
		char a[1000];
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;	
		res = (filler(buf, de->d_name, &st, 0));
			if(res!=0) break;

	}

	closedir(dp);
	return 0;
}

static int E4_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;
  int fd = 0 ;

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int E4_open(const char *path, struct fuse_file_info *fi)
{
	int res;
	char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}
static int E4_truncate(const char *path, off_t size)
{
	int res;
	char dpath[1000];
	char cmd[1000];
	sprintf(dpath, "%s/simpanan", dirpath);
	mkdir(dpath, 0755);
	char *fname;
	fname = strrchr(path, '/');
	sprintf(dpath,"%s%s",dpath, fname);
	char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	sprintf(cmd, "cp %s %s",fpath, dpath);
	system(cmd);
	res = truncate(dpath, size);
	if(res == -1)
		return -errno;
	return 0;
}

static int E4_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
	char fpath[1000];
	char dpath[1000];
	sprintf(dpath, "%s/simpanan", dirpath);
	mkdir(dpath, 0755);
	char *fname;
	fname = strrchr(path, '/');
	sprintf(dpath,"%s%s",dpath, fname);
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	
	(void) fi;
	fd = open(dpath, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static struct fuse_operations E4_oper = {
	.getattr	= E4_getattr,
	.readdir	= E4_readdir,
	.read		= E4_read,
	.truncate	= E4_truncate,
	.open 		= E4_open,
	.write 		= E4_write,
};


int main(int argc, char *argv[])
{
	char currdir[2048];
  	char homedir[2048];
  	getcwd(currdir, sizeof(currdir));
  	chdir(getenv("HOME"));
 	getcwd(homedir, sizeof(homedir));
  	chdir(currdir);
	sprintf(dirpath, "%s/Downloads", homedir);
	umask(0);
	return fuse_main(argc, argv, &E4_oper, NULL);
}
