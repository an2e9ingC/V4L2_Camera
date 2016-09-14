#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "qt.h"
#include "ipc.h"
#include "m0.h"

ssize_t bytes, nread;

extern int shmid_cam;
extern shared_cam_t*shm_cam;	//camera
extern int shmid_m0;
extern shared_cam_t* shm_m0;

int writen(int fd, const void *vptr, int n)
{
	int nleft;
	int nwritten;
	const char *ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ((nwritten = write(fd, ptr, nleft)) <= 0)
		{
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;
			else
				return -1;
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return (n - nleft);
}


void subprocess(int fd)
{
	char order[9];//store the command from the client

	printf("a new process\n");
	//	ssize_t n, m = 0;
	char buf[1024*1024];
	char pic_len[20];
	while (1)
	{
		bzero(order, sizeof(order));
		nread = read(fd, order, sizeof(order));
		order[8] = '\0';
		pid_t pid;

		printf("]]]]]]]]]]]]]]]]]]]]]]]]]%s[[[[[[[[[[[[[[[[[[[[[[[[[[[[\n", order);
		if (0 == strncmp("camon", order, 5))
		{
			printf("order: camon\n");
			pid = fork();
			if(0 == pid)
			{
			sprintf(pic_len, "%d", shm_cam->shm_len);

			writen(fd, "cam", strlen("cam"));
			writen(fd, pic_len, sizeof(pic_len));
			writen(fd, shm_cam->shm_sp, shm_cam->shm_len);

			printf(">>>>>>>>>>%s<<<<<<<<<\n", buf);
			memset(buf, 0, sizeof(buf));
				}
		}
		else if (0 == strncmp("temp", order, 4))
		{
			printf("order: temp\n");
			pid = fork();
			if(0 == pid)
			{
				printf("fork success!\n");
				writen(fd, shm_m0->shm_sp, shm_m0->shm_len);
				printf("????????????????????????%s??????????????????????\n", shm_m0->shm_sp);
			}
		}
		else if (0 == strncmp("ledon", order, 5))
		{
			printf("order: ledon\n");
			pid = fork();
			if(0 == pid)
			{
				printf("fork success!\n");
				int serial_fd = 0;
				serial_fd = init_serial();
				led_on(&serial_fd);

				memcpy(shm_m0->shm_sp, "led-on", 7);
				shm_m0->shm_len = strlen("led_on");
				writen(fd, shm_m0->shm_sp, shm_m0->shm_len);
				printf("!!!!!!!!!!%s!!!!!!!!!!\n", shm_m0->shm_sp);
			}
		}
		else if (0 == strncmp("ledoff", order, 6))
		{
			printf("order: ledoff\n");
			pid = fork();
			if(0 == pid)
			{
				printf("fork success!\n");
				int serial_fd = 0;
				serial_fd = init_serial();
				led_off(&serial_fd);
				memcpy(shm_m0->shm_sp, "led-off", 8);
				shm_m0->shm_len = strlen("led_off");
				writen(fd, shm_m0->shm_sp, shm_m0->shm_len);

				printf("!!!!!!!!!!%s!!!!!!!!!!\n", shm_m0->shm_sp);
			}
		}
		else if (0 == strncmp("fanon", order, 5))
		{
			printf("order: fanon\n");
			pid = fork();
			if(0 == pid)
			{
				printf("fork success!\n");
				int serial_fd = 0;
				serial_fd = init_serial();
				fan_on(&serial_fd);

				memcpy(shm_m0->shm_sp, "fan-on", 7);
				shm_m0->shm_len = strlen("fan-on");
				writen(fd, shm_m0->shm_sp, shm_m0->shm_len);

				printf("!!!!!!!!!!%s!!!!!!!!!!\n", shm_m0->shm_sp);
			}
		}
		else if (0 == strncmp("fanoff", order, 6))
		{
			printf("order: fanoff\n");
			pid = fork();
			if(0 == pid)
			{
				printf("fork success!\n");
				int serial_fd = 0;
				serial_fd = init_serial();
				fan_off(&serial_fd);

				memcpy(shm_m0->shm_sp, "fan-off", 8);
				shm_m0->shm_len = strlen("fan-off");
				writen(fd, shm_m0->shm_sp, shm_m0->shm_len);

				printf("!!!!!!!!!!%s!!!!!!!!!!\n", shm_m0->shm_sp);
			}
		}
		else if (0 == strncmp("beeon", order, 5))
		{
			printf("order: beeon\n");
			pid = fork();
			if(0 == pid)
			{
				printf("fork success!\n");
				int serial_fd = 0;
				serial_fd = init_serial();
				speaker_on(&serial_fd);

				memcpy(shm_m0->shm_sp, "bee-on", 7);
				shm_m0->shm_len = strlen("bee-on");
				writen(fd, shm_m0->shm_sp, shm_m0->shm_len);

				printf("!!!!!!!!!!%s!!!!!!!!!!\n", shm_m0->shm_sp);
			}
		}
		else if (0 == strncmp("beeoff", order, 6))
		{
			printf("order: beeoff\n");
			pid = fork();
			if(0 == pid)
			{
				printf("fork success!\n");
				int serial_fd = 0;
				serial_fd = init_serial();
				speaker_off(&serial_fd);

				memcpy(shm_m0->shm_sp, "bee-off", 8);
				shm_m0->shm_len = strlen("bee-off");
				writen(fd, shm_m0->shm_sp, shm_m0->shm_len);

				printf("!!!!!!!!!!%s!!!!!!!!!!\n", shm_m0->shm_sp);
			}
		}
		else
		{
			printf("order not found\n");
			break;
		}
	}
	close(fd);
}

int tcp_create_socket()
{
	int listenfd;
	socklen_t len;
	struct sockaddr_in servaddr;

	len = sizeof(struct sockaddr_in);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(8000);

	if (0 > (listenfd = socket(AF_INET, SOCK_STREAM, 0)))
	{
		perror("socket");
		return -1;
	}

	unsigned int re_use_addr = 0x01;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&re_use_addr, sizeof(re_use_addr));

	if (0 > bind(listenfd, (struct sockaddr *)&servaddr, len))
	{
		perror("bind");
		return -1;
	}

	if (0 > listen(listenfd, 20))
	{
		perror("listen");
		return -1;
	}

	return listenfd;
}

int qt_task()
{
	int listenfd, sockfd;
	socklen_t len;
	struct sockaddr_in clientaddr;
	pid_t pid;

	len = sizeof(struct sockaddr_in);

	if (-1 == (listenfd = tcp_create_socket()))
	{
		return -1;
	}

	while (1)
	{
		printf("server waiting...\n");
		if (0 > (sockfd = accept(listenfd, (struct sockaddr *)&clientaddr, &len)))
		{
			perror("accept");
			return -1;
		}

		if (0 == (pid = fork()))
		{
			close(listenfd);
			subprocess(sockfd);
			close(sockfd);
			break;
		}
		else if (pid > 0)
		{
			close(sockfd);
			continue;
		}
		else
		{
			printf("fork error\n");
			return -1;
		}
	}
	close(listenfd);
	return 0;
}
