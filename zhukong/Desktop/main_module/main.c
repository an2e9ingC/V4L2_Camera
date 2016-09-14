#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/prctl.h>

#include "ipc.h"
#include "cam.h"
#include "web.h"
#include "m0.h"
#include "qt.h"
#include "lcd.h"
#include "gsm.h"

int shmid_m0 = -1;
shared_m0_t *shm_m0 = NULL;

int shmid_cam = -1;
shared_cam_t *shm_cam = NULL;

pid_t pid_cam, pid_web, pid_zgb, pid_qt, pid_lcd, pid_gsm;
char buf[1024];	//

void sigchld_handler(int signo)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[])
{
	signal(SIGCHLD,sigchld_handler);

	/*create camera share memory*/
	shm_cam = (shared_cam_t*)get_shm(SHM_KEY_CAM, SHM_CHAR_CAM, SHM_SZ_CAM, IPC_CREAT | 0666, shmid_cam);
	if (!shm_cam)
	{
		printf("create camera share memory failed!\n");
		exit(EXIT_FAILURE);
	}
	printf("create camera share memory success!\n");

	/*create m0 share memory*/
	shm_m0= (shared_m0_t*)get_shm(SHM_KEY_M0, SHM_CHAR_M0, SHM_SZ_M0, IPC_CREAT | 0666, shmid_m0);
	if (!shm_m0)
	{
		printf("create m0 share memory failed!\n");
		exit(EXIT_FAILURE);
	}
	printf("create m0 share memory success!\n");


	/*=============camera process===============*/
	pid_cam = fork();
	if (-1 == pid_cam)
	{
		perror("fork_cam");
		goto out;
	} else if (0 == pid_cam)
	{
		cam_task();
	} else
	{
		printf("fork camera process ok, pid = %d\n", pid_cam);
		/*=============qt process===============*/
		pid_qt = fork();
		if (-1 == pid_qt)
		{
			perror("fork_Qt");
			goto out;
		} else if (0 == pid_qt)
		{
			if (-1 == qt_task())
			{
				printf("excute qt_task failed!\n");
				exit(EXIT_FAILURE);
			}
		} else
		{
			printf("fork qt process ok, pid = %d\n", pid_qt);
			/*=============gsm process===============*/
			pid_gsm = fork();
			if (-1 == pid_gsm)
			{
				perror("fork_gsm");
				goto out;
			} else if (0 == pid_gsm)
			{
				gsm_task();
			} else
			{
				printf("fork gsm process ok, pid = %d\n", pid_gsm);
				/*=============lcd process===============*/
				pid_lcd = fork();
				if (-1 == pid_lcd)
				{
					perror("fork_lcd");
					goto out;
				} else if (0 == pid_lcd)
				{
					lcd_task();
				} else
				{
					printf("fork lcd process ok, pid = %d\n", pid_lcd);
					/*=============web process===============*/
					pid_web = fork();
					if (-1 == pid_web)
					{
						perror("fork_web");
						goto out;
					} else if (0 == pid_web)
					{
						web_task();
					} else
					{
						printf("fork web process ok, pid = %d\n", pid_web);
						/*=============zgb process===============*/
						pid_zgb = fork();
						if (-1 == pid_zgb)
						{
							perror("fork_zgb");
							goto out;
						} else if (0 == pid_zgb)
						{
							m0_task();
						} else
						{
							printf("fork zdb process ok, pid = %d\n", pid_zgb);
						}
					}
				}
			}
		}
	}
//	signal(SIGCHLD, sigchld_handler);
out:
	del_shm(shmid_cam, shm_cam);

	return 0;
}
