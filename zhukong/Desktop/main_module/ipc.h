#ifndef IPC_H_
#define IPC_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define ERR (-1)
#define OK 0


#define SEM_KEY "."
#define SEM_CHAR 's'

#define SHM_KEY_CAM "."
#define SHM_SZ_CAM ((128 + 4) * 1024)
#define SHM_CHAR_CAM 'c'

#define SHM_KEY_M0 "."
#define SHM_SZ_M0 ((128 + 4) * 1024)
#define SHM_CHAR_M0 'm'

#define SHM_CAM_SIZE 1024*50
#define SHM_M0_SIZE 36

//M0
typedef struct shared_M0{
	int end_flag;
	int shm_len;
	unsigned char shm_sp[SHM_M0_SIZE]; 
}shared_m0_t;

//camera
typedef struct shared_cam{
	int end_flag;
	int shm_len;
	unsigned char shm_sp[SHM_CAM_SIZE];
}shared_cam_t;


int get_sem(const char *pathname, int sem_char, int sem_flg, int semid);
void *get_shm(const char *pathname, int shm_char, ssize_t size, int shm_flg, int shmid);
int sem_destroy(int semid);
void *del_shm(int shmid, const void *shm);

union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};

#endif