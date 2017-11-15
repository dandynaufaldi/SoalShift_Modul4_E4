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
int nemu = 0;
static int check(const char *path)
{
	char *tmp = strrchr(path, '.');
	if (!tmp) return 0;
	if (strcmp(tmp, ".copy")==0) return 1;
	return 0;
}

static int E4_getattr(const char *path, struct stat *stbuf)
{
	// if (strcmp(path, ".")!=0 && strcmp(path, "..")!=0 && check(path)==1){
	// 	printf("nemu %d\n", nemu++);
	// 	char command[2048];
	// 	//sprintf(command, "notify-send \"File yang anda buka adalah file hasil salinan. File tidak bisa diubah maupun disalin kembali!\"");
	// 	sprintf(command, "zenity --error --text=\"File yang anda buka adalah file hasil salinan. File tidak bisa diubah maupun disalin kembali!\"");
	// 	system(command);
	// 	//return 0;
	// }
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);
	res = lstat(fpath, stbuf);
	if (res == -1)
		return -errno;
	
	return 0;
}

static int E4_chmod(const char *path, mode_t mode)
{
	int res;

	res = chmod(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int E4_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
// if (strcmp(path, ".")!=0 && strcmp(path, "..")!=0 && check(path)==1){
// 		char command[2048];
// 		//sprintf(command, "notify-send \"File yang anda buka adalah file hasil salinan. File tidak bisa diubah maupun disalin kembali!\"");
// 		sprintf(command, "zenity --error --text=\"File yang anda buka adalah file hasil salinan. File tidak bisa diubah maupun disalin kembali!\"");
// 		system(command);
// 		//return 0;
// 	}
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
	// if (strcmp(path, ".")!=0 && strcmp(path, "..")!=0 && check(path)==1){
	// 	printf("nemu %d\n", nemu++);
	// 	char command[2048], command2[2048];
	// 	sprintf(command, "notify-send \"File %s yang anda buka adalah file hasil salinan. File tidak bisa diubah maupun disalin kembali!\"", path);
	// 	//sprintf(command, "zenity --error --text=\"File yang anda buka adalah file hasil salinan. File tidak bisa diubah maupun disalin kembali!\"");
	// 	system(command);
	// 	// sprintf(command2, "chmod 444 \"%s\"", path);
	// 	// system(command2);
	// 	E4_chmod(path, S_IRUSR|S_IRGRP|S_IROTH);
	// 	//return 0;
	// }
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
	if (strcmp(path, ".")!=0 && strcmp(path, "..")!=0 && check(path)==1){
		printf("nemu %d\n", nemu++);
		char *tmppath = path+1;
		char command[2048], command2[2048];
		sprintf(command, "notify-send \"File yang anda buka adalah file hasil salinan. File tidak bisa diubah maupun disalin kembali!\"");
		//sprintf(command, "zenity --error --text=\"File yang anda buka adalah file hasil salinan. File tidak bisa diubah maupun disalin kembali!\"");
		system(command);
		sprintf(command2, "chmod 000 %s", tmppath);
		system(command2);
		//E4_chmod(path, S_IRUSR|S_IRGRP|S_IROTH);
		return -errno;
	}
	int res;

	res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}


static int E4_create(const char* path, mode_t mode, struct fuse_file_info* fi) {

    (void) fi;

    int res;
    res = creat(path, mode);
    if(res == -1)
	return -errno;

    close(res);

    return 0;
}


static struct fuse_operations E4_oper = {
	.getattr	= E4_getattr,
	.readdir	= E4_readdir,
	.read		= E4_read,
	.create     = E4_create,
	.open		= E4_open,
	.chmod		= E4_chmod,
};


int main(int argc, char *argv[])
{
	char currdir[2048];
  	char homedir[2048];
  	getcwd(currdir, 99);
  	chdir(getenv("HOME"));
 	getcwd(homedir, 99);
  	chdir(currdir);
	sprintf(dirpath, "%s/Downloads", homedir);
	umask(0);
	return fuse_main(argc, argv, &E4_oper, NULL);
}
