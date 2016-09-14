#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "cam.h"
#include "ipc.h"

extern int shmid_cam;
extern shared_cam_t* shm_cam;

char buf[1024*1024];

void cam_task()
{
	printf("cam task\n");
	ssize_t n, m = 0;
/*
	int fd = open("1.jpg", O_RDONLY);
	if(0 > fd)
	{
		perror("open");
		exit(EXIT_FAILURE);
	}

	
	char buf[1024*11] = {0};//final
	char content[1024*10] = {0};//file_contents	
	char head[3] = "CAM";//head_flag
	
	while((bytes = read(fd, content, sizeof(content))) > 0)by += bytes;//read_file

	strcat(buf, head);//buf+head
	strcat(buf, content);//buf+head+content
*/
	pid_t pid = fork();
	if(0 == pid)
	{
		while(1)
		{
			int fp = open("pic1.jpg", O_RDONLY);
			if(0 > fp)
			{
				perror("open");
				exit(EXIT_FAILURE);
			}
								
			while((n = read(fp, buf + m, sizeof(buf))) > 0)
			{
				m += n;
			}
			shm_cam->shm_len = m;
			memcpy(shm_cam->shm_sp, buf, shm_cam->shm_len);
		}
	}
	
}
