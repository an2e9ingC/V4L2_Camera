#ifndef _CAMERA_H
#define _CAMERA_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/videodev2.h>
#include <errno.h>
#include <sys/mman.h>

#define PC_DEBUG   1// enable/disable debug on PC
#define USE_FILE   2 // Enale / Disable use file to convert pictures
#define JPEG_COMPRESS   3
#define BMP_HEADER  4

/* definations for captureing pictures*/
#define REQ_BUFF_COUNT 5
#define WIDTH 640
#define HEIGHT 480

/* definations for JPEG/bmp  compression&encode*/
#ifdef BMP_HEADER
#define WORD unsigned short
#define DWORD unsigned long
typedef struct tagBITMAPFILEHEADER{
    WORD    bfType;                // the flag of bmp, value is "BM"
    DWORD    bfSize;                // size BMP file ,unit is bytes
    DWORD    bfReserved;            // 0
    DWORD    bfOffBits;             // must be 54

}bit_map_file_header;

typedef struct tagBITMAPINFOHEADER{
    DWORD    biSize;                // must be 0x28
    DWORD    biWidth;           //
    DWORD    biHeight;          //
    WORD     biPlanes;          // must be 1
    WORD     biBitCount;            //
    DWORD    biCompression;         //
    DWORD    biSizeImage;       //
    DWORD    biXPelsPerMeter;   //
    DWORD    biYPelsPerMeter;   //
    DWORD    biClrUsed;             //
    DWORD    biClrImportant;        //
}bit_map_info_header;
#endif

#ifdef JPEG_COMPRESS
#include "setjmp.h"
#include "jpeglib.h"
#include "sys/types.h"	//size_t
#include "jerror.h"
#define JPEG_QUALITY 50 //the quality of JPEG
JSAMPLE * image_buffer;	/* Points to large array of R,G,B-order data */

#endif

#ifdef PC_DEBUG
extern char*  device_fname;
extern int file_count;
#else
extern char*  device_fname;
#endif

/*files that may be used while dealing pictures
 * but when use share memory, these are useless.
 */
#ifdef USE_FILE
extern char*  output_fname_rgb_bmp;
extern char*  output_fname_jpg;
#endif

extern int yuyv_out_fd;    //file description of out put .yuyv file
extern char*  output_fname_yuyv; //yuyv file name
extern int camera_fd; // file description of camera
extern struct v4l2_capability cap;  //capabilities of device
typedef struct _buffer  //定义一个结构体来映射每个缓冲帧
{
    void* start;    //起始地址
    unsigned int length;    //帧长度
}single_buff;

extern single_buff* frame_buf; //(用户空间)记录帧 映射到用户空间(包含4个用户可用的帧)

/**req_buffers: 内核视频缓冲区VIDIOC_REQBUFS,原始数据
    (3个以上, 每个存放一帧图像)
    1.  app通过VIDIOC_QUERYBUF可以查询到缓冲区在内核中的长度和偏移量地址;
    2.  用户访问缓冲区需要 地址映射 mmap() 到用户空间才能访问;
*/
extern struct v4l2_requestbuffers req_buffers;


int open_camera_file(const char* const file_name);  //open camera
void  check_support_fmt(void);  //VIDIOC_ENUM_FMT // 查询,显⽰所有⽀持的格式
void check_device_info(void);   //获取设备信息,支持的格式
void get_current_frame_fmt(void);   //查看当前视频帧格式: VIDIOC_G_FMT
int set_capture_frame_fmt(void);    //设置捕捉的 帧格式(视频制式NTSC/PAL????)
int request_buffers(int cnt);   //申请视频流缓冲区, 包含cnt个缓存
int memory_map(void);   //映射地址, 获取缓冲帧的地址和长度
int start_capture(void);    //循环采集数据
int save_to_yuyv(void); //处理采集到的帧
int yuyv_to_rgb_pixel(int y, int u, int v); //根据输入的y,u,v分量,把yuyv转化为rgb格式的像素, 返回一个32bit像素的图像
int yuyv_to_rgb_buffer(unsigned char*yuv, unsigned char*rgb,
    unsigned int width, unsigned int height);
//根据YUYV图片的一个像素信息yuv(4byte)和图片的width,height,放到rgb的缓存rgb中.

void set_bmp_header(bit_map_file_header *bf, bit_map_info_header *bi);  //Set bit_map_info_header
int yuyv_to_rgb24(void);    // 把yuyv格式的图片转换成RGB24格式
int stop_capture(void); // 停止采集
void close_camera(void);
int device_init(void);
void write_JPEG_file (char* bmp_filename, char* jpg_filename, int quality); //根据bmp文件和quality, 生成jpg文件jpg_filename);

#endif
