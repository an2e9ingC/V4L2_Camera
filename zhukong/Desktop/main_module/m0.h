#ifndef M0_H_
#define M0_H_

extern unsigned char snd[36];

#define DEVICE "/dev/ttyUSB0"
int  init_serial(void);
int uart_send(int fd, char *data, int datalen) ;
int uart_recv(int fd, char *data, int datalen) ;

void led_on(int *fd);
void led_off(int *fd);
void fan_on(int *fd);
void fan_off(int *fd);
void speaker_on(int *fd);
void speaker_off(int *fd);

int uart_send(int fd, char *data, int datalen);

void m0_task();

#endif