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
#include <string.h>

char dirpath[100];


static int xmp_getattr(const char *path, struct stat *stbuf)
{
  int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);
	res = lstat(fpath, stbuf);
	if (res == -1)
		return -errno;
	

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
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

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
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
	char temp[5];
	for(int i=4;i>0;i--)
	{
		temp[4-i] = fpath[strlen(fpath) - i];
	}
	temp[4] ='\0';
	if((strcmp(temp, ".txt") == 0) || (strcmp(temp, ".doc") == 0) || (strcmp(temp, ".pdf") == 0))
	{
		char zpath[1000];
		sprintf(zpath,"%s.ditandai",fpath);
		rename(fpath, zpath);
		system("zenity --error --text=\"Terjadi Kesalahan! File berisi konten berbahaya.\n\" --title=\"ERROR\"");
		return -errno;
	}
	else
	{
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
	return res;


}



static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
};



int main(int argc, char *argv[])
{
	char currdir[100];
  	char homedir[100];
  	getcwd(currdir, 99);
  	chdir(getenv("HOME"));
 	getcwd(homedir, 99);
  	chdir(currdir);
	sprintf(dirpath, "%s/Documents", homedir);
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
