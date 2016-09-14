#include "camera.h"
#include "shm.h"
#define BUFF_SIZE 1024*1000  //50kB

#ifdef PC_DEBUG
char* device_fname = "/dev/video1";
#else
char *device_fname = "/dev/video0";
#endif

char*  output_fname_yuyv = "./pic/pic1.yuyv"; //yuyv file name
char *output_fname_rgb_bmp = "./pic/pic1.bmp";
char *output_fname_jpg = "./pic/pic1.jpg";
int yuyv_out_fd = -1;    //file description of out put .yuyv file
int camera_fd = -1; // file description of camera
single_buff* frame_buf= NULL; //(用户空间)记录帧 映射到用户空间(包含4个用户可用的帧)
struct v4l2_requestbuffers req_buffers;
struct v4l2_capability cap;  //capabilities of device

void err_exit(char* process_name)
{
    perror(process_name);
    exit(-1);
}

void old_deal()
{
    /*1. open video0 device*/
    camera_fd = open_camera_file(device_fname);
    if(camera_fd < 0)
        err_exit("open_camera_file");

    /*2. 查看设备能力,是否有音视频输入输出?(VIDIOC_QUERYCAP：查询驱动功能)*/
    check_device_info();

    // /*3. 设备初始化*/
    if(device_init() < 0)
        err_exit("device_init");
    /*4. 开始获取图像*/
    if( start_capture() < 0)
        err_exit("start_capture");
    // while (1) {
        /*5. 处理图片*/
        save_to_yuyv();
        yuyv_to_rgb24();
        // rgb24_to_shm(shm_addr); //Do NOT put the rgb stream into share-memory instead of files
        write_JPEG_file(output_fname_rgb_bmp, output_fname_jpg, JPEG_QUALITY);

    // }
    /*6. 关闭设备*/
    close_camera();

}

int main(int argc, char* argv[])
{
    pid_t pid;  //进程id
    key_t key;
    int shmid;  //share memory id
    char* shm_addr;
    char *flag = "JPG"; //标志头
    char buff[BUFF_SIZE];

    //创建共享内存
    if((key = ftok(".", 'c')) == -1){
        err_exit("ftok");
        printf("key = %d\n", key);
    }
    printf("ftok ok...key = %d\n", key);
    if( (shmid = shmget(key, BUFF_SIZE, 0666|IPC_CREAT)) < 0){
        printf("shmid = %d\n", shmid);
        err_exit("shmget");
    }else{
        printf("shmget ok. shmid = %d\n", shmid);
    }
    //show the share memory status
    system("ipcs -m");

    pid = fork();
    if(pid < 0){
        err_exit("fork");
    }
    if(0 == pid){   // child process
        shm_addr = shmat(shmid, 0, 0);
        if( shm_addr == (void *)-1 ){
            shmdt(shm_addr);
            err_exit("shmat");
        }else{
            printf("------Child: Attach share-memory ok: %p\n", shm_addr);
        }
        system("ipcs -m");
        /*put the jpg data stream to the share-memory*/
        /*1. open video0 device*/
        camera_fd = open_camera_file(device_fname);
        if(camera_fd < 0)
            err_exit("open_camera_file");

        /*2. 查看设备能力,是否有音视频输入输出?(VIDIOC_QUERYCAP：查询驱动功能)*/
        check_device_info();

        // /*3. 设备初始化*/
        if(device_init() < 0)
            err_exit("device_init");
        /*4. 开始获取图像*/
        if( start_capture() < 0)
            err_exit("start_capture");
         while (1) {
            /*5. 处理图片*/
            save_to_yuyv();
            // yuyv_to_rgb24();
            rgb24_to_shm(shm_addr); //Do NOT put the rgb stream into share-memory instead of files
            // write_JPEG_file(output_fname_rgb_bmp, output_fname_jpg, JPEG_QUALITY);

			   shmdt(shm_addr);
         }
        /*6. 关闭设备*/
        close_camera();
        printf("-----Child process End...\n");

    }
    else    //parent process
    {
        shm_addr = shmat(shmid, 0, 0);
        if( shm_addr == (void *)-1 ){
            err_exit("shmat");
        }else{
            printf("----Parent: Attach share-memory ok: %p\n", shm_addr);
        }
        system("ipcs -m");
        sleep(1);

        char buf[BUFF_SIZE];
        strncpy(shm_addr, buf, BUFF_SIZE);
        printf("shm_addr---------\n %p \n", shm_addr);
        printf("Parent process End...\n");
    }



//     old_deal();
    return 0;
}
