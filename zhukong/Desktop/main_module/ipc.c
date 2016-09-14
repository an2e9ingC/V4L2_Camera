#include "ipc.h"
#include <stdio.h>

int get_sem(const char *pathname, int sem_char, int sem_flg, int semid)
{
	key_t sem_key;
	int ret;
	union semun semun;

	/*create a key*/
	sem_key = ftok(pathname, sem_char);
	if (-1 == sem_key)
	{
		perror("semaphore_ftok");
		return ERR;
	}

	/*create a semaphore*/
	semid = semget(sem_key, 1, sem_flg);
	if (-1 == semid)
	{
		perror("semaphore_semget");
		return ERR;
	}

	semun.val = 1;
	ret = semctl(semid, 0, SETVAL, semun);
	if (-1 == ret)
	{
		perror("semctl");
		ret = semctl(semid, 0, IPC_RMID, semun);
		if (-1 == ret)
		{
			perror("semaphore_semctl");
			return ERR;
		}
		return ERR;
	}
	return OK;
}

int sem_destroy(int semid)
{
	int ret;
	union semun semun;

	ret = semctl(semid, 0, IPC_RMID, semun);
	if (-1 == ret) {
		perror("semaphore");
		return ERR;
	}

	return OK;
}

void *get_shm(const char *pathname, int shm_char, ssize_t size, int shm_flg, int shmid)
{
	key_t shm_key;
	int ret;
	void *shm = NULL;

	/*create a key*/
	shm_key = ftok(pathname, shm_char);
	if (-1 == shm_key)
	{
		perror("share_memory_ftok");
		return NULL;
	}

	/*create a share_memory*/
	shmid = shmget(shm_key, size, shm_flg);
	if (-1 == shmid)
	{
		perror("share_memory_shmget");
		return NULL;
	}

	shm = shmat(shmid, NULL, 0);
	if (!shm)
	{
		perror("shmat");
		ret = shmctl(shmid, IPC_RMID, NULL);
		if (ret == -1) {
			perror("shared memory");
			return NULL;
		}
		return NULL;
	}

	return shm;
}

//撤销并删除共享内存区域
void *del_shm(int shmid, const void *shm)
{
	//撤销共享内存
	if((shmdt(shm)) < 0)
	{
		perror("shmdt");
		return NULL;
	}

	//删除
	shmctl(shmid, IPC_RMID, NULL);
	return NULL;
}