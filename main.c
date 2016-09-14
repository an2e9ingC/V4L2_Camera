#include "camera.h"

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

int main(int argc, char* argv[])
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


    while (1) {
        /*5. 处理图片*/
        save_to_yuyv();
        yuyv_to_rgb24();
        write_JPEG_file(output_fname_rgb_bmp, output_fname_jpg, JPEG_QUALITY);
    }


    /*6. 关闭设备*/
    close_camera();

    return 0;
}
