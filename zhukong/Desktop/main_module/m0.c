#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> //\u6587\u4ef6\u63a7\u5236\u5b9a\u4e49  
#include <termios.h>//\u7ec8\u7aef\u63a7\u5236\u5b9a\u4e49  
#include <errno.h>

#include "m0.h"
#include "ipc.h"

extern int shmid_m0;
extern shared_m0_t* shm_m0;
unsigned char snd[36] = {0XDD, 0X04, 0X24, 0X00, 0X01};


int init_serial(void)
{
    int serial_fd = 0;
    serial_fd = open(DEVICE, O_RDWR | O_NOCTTY | O_NDELAY);

    if (serial_fd < 0) {
        perror("open");
        return -1;
    }

    //\u4e32\u53e3\u4e3b\u8981\u8bbe\u7f6e\u7ed3\u6784\u4f53termios <termios.h>
    struct termios options;
    /**1. tcgetattr\u51fd\u6570\u7528\u4e8e\u83b7\u53d6\u4e0e\u7ec8\u7aef\u76f8\u5173\u7684\u53c2\u6570\u3002
    *\u53c2\u6570fd\u4e3a\u7ec8\u7aef\u7684\u6587\u4ef6\u63cf\u8ff0\u7b26\uff0c\u8fd4\u56de\u7684\u7ed3\u679c\u4fdd\u5b58\u5728termios\u7ed3\u6784\u4f53\u4e2d
    */
    tcgetattr(serial_fd, &options);
    /**2. \u4fee\u6539\u6240\u83b7\u5f97\u7684\u53c2\u6570*/
    cfsetispeed(&options, B115200);//\u8bbe\u7f6e\u6ce2\u7279\u7387
    cfsetospeed(&options, B115200);//\u8bbe\u7f6e\u6ce2\u7279\u7387
    options.c_cflag |= (CLOCAL | CREAD);//\u8bbe\u7f6e\u63a7\u5236\u6a21\u5f0f\u72b6\u6001\uff0c\u672c\u5730\u8fde\u63a5\uff0c\u63a5\u6536\u4f7f\u80fd
    options.c_cflag &= ~CSIZE;//\u5b57\u7b26\u957f\u5ea6\uff0c\u8bbe\u7f6e\u6570\u636e\u4f4d\u4e4b\u524d\u4e00\u5b9a\u8981\u5c4f\u6389\u8fd9\u4e2a\u4f4d
    options.c_cflag |= CS8;//8\u4f4d\u6570\u636e\u957f\u5ea6
    options.c_cflag &= ~CRTSCTS;//\u65e0\u786c\u4ef6\u6d41\u63a7
    options.c_cflag &= ~CSTOPB;//1\u4f4d\u505c\u6b62\u4f4d
    // options.c_iflag |= IGNPAR;//\u65e0\u5947\u5076\u68c0\u9a8c\u4f4d
    //\u65e0\u5947\u5076\u6821\u9a8c\u4f4d\u3002
    options.c_cflag &= ~PARENB;
    options.c_iflag &= ~INPCK;

    options.c_oflag &= ~OPOST; //\u4fee\u6539\u8f93\u51fa\u6a21\u5f0f\uff0c\u539f\u59cb\u6570\u636e\u8f93\u51fa
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    options.c_oflag = 0; //\u8f93\u51fa\u6a21\u5f0f
    options.c_lflag = 0; //\u4e0d\u6fc0\u6d3b\u7ec8\u7aef\u6a21\u5f0f
    /**3. \u8bbe\u7f6e\u65b0\u5c5e\u6027\uff0cTCSANOW\uff1a\u6240\u6709\u6539\u53d8\u7acb\u5373\u751f\u6548*/
    tcflush(serial_fd, TCIFLUSH);//\u6ea2\u51fa\u6570\u636e\u53ef\u4ee5\u63a5\u6536\uff0c\u4f46\u4e0d\u8bfb

    if (tcsetattr(serial_fd, TCSANOW, &options) != 0)
    {
        perror("tcsetattr error");
        return -1;
    }

    printf("configure complete\n");

    return serial_fd;

}
int uart_send(int fd, char *data, int datalen)
{
    int len = 0;
    len = write(fd, data, datalen);//\u5b9e\u9645\u5199\u5165\u7684\u957f\u5ea6
    if (len == datalen) {
        return len;
    } else {
        tcflush(fd, TCOFLUSH);//TCOFLUSH\u5237\u65b0\u5199\u5165\u7684\u6570\u636e\u4f46\u4e0d\u4f20\u9001
        return -1;
    }

    return 0;

}


struct timeval tv_timeout;

int uart_recv(int fd, char *data, int datalen)
{

    int len = 0, ret = 0;
    fd_set fs_read;

    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);
    //tv_timeout.tv_sec  = (10*20/115200+2);
    // tv_timeout.tv_usec = 0;

    // ret = select(fd+1, &fs_read, NULL, NULL, &tv_timeout);
    ret = select(fd + 1, &fs_read, NULL, NULL, NULL);
    printf("ret = %d\n", ret);
    //\u5982\u679c\u8fd4\u56de0\uff0c\u4ee3\u8868\u5728\u63cf\u8ff0\u7b26\u72b6\u6001\u6539\u53d8\u524d\u5df2\u8d85\u8fc7timeout\u65f6\u95f4,\u9519\u8bef\u8fd4\u56de-1



    if (FD_ISSET(fd, &fs_read))
    {
        len = read(fd, data, datalen);
        printf("len = %d\n", len);
        return len;
    }
    else
    {
        perror("select");
        return -1;
    }
}
void led_on(int *fd)
{
    int serial_fd = *fd;
    snd[4] = 0x00;
    uart_send( serial_fd, (char *)snd, 36) ;
	printf("send ok!\n");
}
void led_off(int *fd)
{
    int serial_fd = *fd;
    snd[4] = 0x01;
    uart_send( serial_fd, (char *)snd, 36) ;
}

void fan_on(int *fd)
{
    int serial_fd = *fd;
    snd[4] = 0x04;
    uart_send( serial_fd, (char *)snd, 36) ;
}
void fan_off(int *fd)
{
    int serial_fd = *fd;
    snd[4] = 0x08;
    uart_send( serial_fd, (char *)snd, 36) ;
}
void speaker_on(int *fd)
{
    int serial_fd = *fd;
    snd[4] = 0x02;
    uart_send( serial_fd, (char *)snd, 36) ;
}
void speaker_off(int *fd)
{
    int serial_fd = *fd;
    snd[4] = 0x03;
    uart_send( serial_fd, (char *)snd, 36) ;
}

void m0_task()
{
	int i, j;
	unsigned	char buf[50];
	int serial_fd = init_serial();

	led_on(&serial_fd);
	fan_on(&serial_fd);

	//sleep(7);
	//fan_off(&serial_fd);
	//led_off(&serial_fd);

	pid_t pid = fork();
	if(0 == pid)
	{
	while (1)
	{
		memset(buf, 0, 50);
		uart_recv(serial_fd, (char *)buf, 50);
		for (j = 0; j < 50; j++)
		{
			if (buf[j] == 0xbb)
			{
				for (i = 0; i < 36; i++)
				{
					printf("%.2x  ", buf[i]);
				}
			}
		}
		printf("\n");
		

		int temp = ((buf[4] << 8) | buf[5]);
		int hum = ((buf[6] << 8) | buf[7]);
		int light = ((buf[21] << 8) | buf[20]);

		char *p1 = (char *)malloc(10);
		char *p2 = (char *)malloc(10);
		char *p3 = (char *)malloc(10);

		sprintf(p1, "%d", temp);
		sprintf(p2, "%d", hum);
		sprintf(p3, "%d", light);
		
		printf("temp=%s\n", p1);
		printf("hum=%s\n",p2);
		printf("light=%s\n", p3);

		char head[20] = "tmp";
		strcat(head, p1);
		strcat(head, p2);
		strcat(head, p3);//tmp+temp+hum+light
		printf("head = %s\n", head);
		shm_m0->shm_len = strlen(head);
		memcpy(shm_m0->shm_sp, head,20 );
		sleep(3);
	}
		}

	close(serial_fd);
}
