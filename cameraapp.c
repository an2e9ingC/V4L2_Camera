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

#define REQ_BUFF_COUNT 5

const char* const device_fname = "/dev/video0";
const char* const output_fname = "catch1";
static int fd = -1; //摄像头文件描述符
typedef struct _buffer  //定义一个结构体来映射每个缓冲帧
{
    void* start;    //起始地址
    unsigned int length;    //帧长度
}single_buff;
single_buff* frame_buf = NULL; //(用户空间)记录帧 映射到用户空间(包含4个用户可用的帧)

/**
    req_buffers: 内核视频缓冲区VIDIOC_REQBUFS,原始数据
    (3个以上, 每个存放一帧图像)
    1.  app通过VIDIOC_QUERYBUF可以查询到缓冲区在内核中的长度和偏移量地址;
    2.  用户访问缓冲区需要 地址映射 mmap() 到用户空间才能访问;
*/
struct v4l2_requestbuffers req_buffers;

void  check_support_fmt(void);

int open_file(const char*const file_name)
{
    int retfd =  open(file_name, O_RDWR,  0);
    if(retfd < 0){
       perror("File open");
       return(-1);
    }
    return retfd;
}

void check_device_info(void)
{//获取设备信息,支持的格式
    printf("Checking Device Info......\n");
    struct v4l2_capability cap;
    ioctl(fd,VIDIOC_QUERYCAP, &cap);
    printf("device name : %s\n", cap.card);
    printf("device driver : %s\n", cap.driver);
    printf("device bus_info : %s\n", cap.bus_info);
    printf("KERNEL_VERSION : %u.%u.%u ;\n", (cap.version>>16)& 0xFF,
                (cap.version>>8)&0XFF, (cap.version>>0) & 0xFF );
    printf("device capabilities : %u\n", cap.capabilities);
    if(V4L2_BUF_TYPE_VIDEO_CAPTURE == cap.capabilities){
        printf("This device supports iamge.\n");
    }else{
        printf("This device does not support iamge.\n");
    }
    check_support_fmt();
}

void  check_support_fmt(void)
{//VIDIOC_ENUM_FMT // 显⽰所有⽀持的格式
    printf("Checking Device Supported Format......\n");
    struct v4l2_fmtdesc fmtdesc;
    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    printf("Supported Formats:\n" );
    while(ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1){
        printf("\t%d.%s\n", fmtdesc.index+1, fmtdesc.description);
        fmtdesc.index++;
    }
    //检查是否支持格式V4L2_PIX_FMT_MJPEG
    struct v4l2_format fmt;
    memset ( &fmt, 0, sizeof(fmt) );
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat=V4L2_PIX_FMT_MJPEG;
    if(ioctl(fd,VIDIOC_TRY_FMT,&fmt)==-1 && errno==EINVAL){
        printf("not support format V4L2_PIX_FMT_MJPEG!\n");
    }else{
        printf("Do support format V4L2_PIX_FMT_MJPEG!\n");
    }

}

void get_current_frame_fmt(void)
{//查看当前视频帧格式: VIDIOC_G_FMT
    printf("Getting Current Frame Format......\n" );
    struct v4l2_format current_fmt;
    memset ( &current_fmt, 0, sizeof(current_fmt) );
    current_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ioctl(fd, VIDIOC_G_FMT, &current_fmt);
    printf("Current Frame Format: \n" );
    printf("\t size of the buffer = %d;\n"
            "\t width=%u;\n\t height=%u;\n",
                current_fmt.fmt.pix.sizeimage,
                current_fmt.fmt.pix.width,
                current_fmt.fmt.pix.height);
    if( current_fmt.fmt.pix.field == V4L2_FIELD_INTERLACED){
         printf("Storate format is interlaced frame format\n");
    }
    printf("Current Format: %ul\n", current_fmt.fmt.pix.pixelformat);

    // struct v4l2_fmtdesc current_fmtdesc;
    // memset ( &current_fmtdesc, 0, sizeof(current_fmtdesc) );
    // current_fmtdesc = current_fmt.fmt.pix.pixelformat;
    // current_fmtdesc.index = 0;
    // current_fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // while (ioctl(fd, VIDIOC_G_FMT, &current_fmtdesc) != -1) {
    //     if( current_fmtdesc.pixelformat & current_fmt.fmt.pix.pixelformat){
    //         printf("Current Format: %s\n", current_fmtdesc.description);
    //     }
    //     current_fmtdesc.index++;
    // }
}

int set_catch_frame_fmt(void)
{//设置捕捉的 帧格式
    printf("Seting Catch Frame fmt ......\n");
    struct v4l2_format fmt; //stream data format
    memset ( &fmt, 0, sizeof(fmt) );
    // definition of an image format( struct v4l2_format.fmt.pix)
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = 640;
    fmt.fmt.pix.height      = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    // fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

    // set fmt
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("set format");
        return -1;    // error return -1
    }
    printf("Seting Catch Frame fmt Ok!\n");
    return 0;   // success return 0;
}


int request_buffers(int cnt)
{//申请视频流缓冲区, 包含cnt个缓存
    memset(&req_buffers, 0, sizeof(req_buffers));

    req_buffers.count = cnt;
    req_buffers.type =  V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req_buffers.memory = V4L2_MEMORY_MMAP;
    if(ioctl(fd, VIDIOC_REQBUFS, &req_buffers) < 0){
        perror("request_buffers: VIDIOC_REQBUFS");
        return -1;
    }
    if (req_buffers.count < 2){
        printf("insufficient buffer memory\n");
        printf("Number of buffers allocated = %d\n", req_buffers.count);
        return -1;
    }
    return 0;
}

int memory_map(void)
{//映射地址, 获取缓冲帧的地址和长度
    printf("Memory Mapping...\n");
    //  1.动态分配数组内存
    // calloc 分配count个缓冲帧, 每个大小为sizeof(*frame_buf), 即8byte(32bit系统)
    frame_buf = (single_buff*) calloc (req_buffers.count, sizeof(*frame_buf));
    if(frame_buf == NULL){
        perror("calloc:");
        return -1;
    }else{
        printf("calloc ok. buffers addr = %#x\n",frame_buf);
    }
    //  2.映射,把所有的缓冲帧都分别映射到用户地址空间
    unsigned int i = 0;
    for(i; i < req_buffers.count; i++){
        struct v4l2_buffer  tmp_buf;    //驱动采集的某一帧
        memset(&tmp_buf, 0, sizeof(tmp_buf));
        tmp_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        tmp_buf.memory = V4L2_MEMORY_MMAP;
        tmp_buf.index = i;

        //查询序号为i的缓冲区, 得到它的起始物理地址和大小,暂存到tmp_buf中
        if( -1 == ioctl(fd, VIDIOC_QUERYBUF, &tmp_buf)){
            perror("Memory Mappping--VIDIOC_QUERYBUF");
            return -1;
        }
        //映射到frame_buf[i].start开始地址,映射到用户空间
        frame_buf[i].length = tmp_buf.length;
        printf("frame_buf[%d].length = %u\n", i, frame_buf[i].length);

        frame_buf[i].start =
            mmap(NULL,
                tmp_buf.length,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                fd,
                tmp_buf.m.offset);
        if( MAP_FAILED == frame_buf[i].start){
            printf("Memory Mappping--mmap failed.\n");
            return -1;
        }
        printf("buffers[%d].start = %#x\n", i, frame_buf[i].start);
        // 保存长度

    }
}


int set_inout_method(void)
{//申请管理缓冲区
    //设置输入输出方法(缓冲区管理): 使用内存映射mmap
    printf("Setting Input/Output Method......\n");
    //1.申请缓冲区(包含REQ_BUFF_COUNT个帧缓冲)
    request_buffers(REQ_BUFF_COUNT);
    //2.获取缓冲帧的地址,长度(通过frame_buf来保存)
    memory_map();
}

int add_to_input_queue(void)
{//把缓冲帧 放入 驱动是视频输入队列
    printf("Adding Frame to Input Queues......\n");
    unsigned int i;
    struct v4l2_buffer  tmp_buf;
    memset(&tmp_buf, 0, sizeof(tmp_buf));
    tmp_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tmp_buf.memory = V4L2_MEMORY_MMAP;
    //加入输入队列
    for(i; i < req_buffers.count; i++){


        tmp_buf.index = i;
        if( ioctl(fd, VIDIOC_QBUF, &tmp_buf) < 0){
            perror("Adding to Input Queues");
            return -1;
        }
    }
    return 0;
}

/**
    采集数据步骤:
    1.  把缓冲区req_buffers(5个)放到视频输入队列 VIDIOC_QBUF;
        输入队列input_queues(等待驱动存放视频的队列)
    2.  启动视频获取,ioctl:VIDIOC_STREAMON;
    3.  循环采集连续视频;
    4.  将输入队列的第一个帧缓冲区 --> 输出队列
        输出队列output_queues(等待用户读取视频的队列)
*/
int start_catch(void)
{//循环采集数据
    printf("Starting Catching Frame......\n");

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret = 0;
    ret  = ioctl(fd, VIDIOC_STREAMON, &type);
    if(ret < 0){
        perror("Starting Catching--VIDIOC_STREAMON");
        return ret;
    }
    return ret;
}

/**
    数据处理:
    1.  从输出队列output_queues取出存有视频的帧缓冲req_buffers[i]: VIDIOC_DQBUF
    2.  处理数据;
    3.  处理完后, 将该帧缓冲req_buffers[i] 重新放回输入队列input_queues: VIDIOC_QBUF
    start_catch(); 和 process_image(); 应放在一起实现循环采集处理
*/
int process_image(void)
{//处理采集到的帧
    printf("Processing Image......\n");
    struct v4l2_buffer  tmp_buf;
    memset(&tmp_buf, 0, sizeof(tmp_buf));
    tmp_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    printf("清空后的buffer类型:%d\n", tmp_buf.type);
    tmp_buf.memory = V4L2_MEMORY_MMAP;
    printf("xxxxxxxxxxxxx......\n");

    ioctl(fd, VIDIOC_DQBUF, &tmp_buf);
    printf("序号:%u\n", tmp_buf.index);
    printf("buffer类型:%d\n", tmp_buf.type);
    printf("bytesused:%u\n", tmp_buf.bytesused);
    printf("flags:%u\n", tmp_buf.flags);
    printf("sequence队列序号:%u\n", tmp_buf.sequence);
    printf("缓冲帧地址 :%u\n", tmp_buf.m.offset);
    printf("缓冲帧length:%u\n", tmp_buf.length);

    int wrfd = open(output_fname, O_WRONLY | O_CREAT | O_TRUNC , 0664);
    if(wrfd < 0){
        perror("output file open");
        return -1;
    }
    ssize_t wrsize = write(wrfd, &tmp_buf, tmp_buf.bytesused);
    printf("wrsize = %.3lf kB\n", wrsize);

    // int i = 4;
    // while(i--){
    //     ioctl(fd, VIDIOC_DQBUF, &tmp_buf);
    //     ioctl(fd, VIDIOC_QUERYBUF, &tmp_buf);
    //     printf("tmp_buf.bytesused = %u Byte\n", tmp_buf.bytesused);
    //     // //处理数据
    //     int wrfd = open("pic1.jpg", O_WRONLY | O_CREAT | O_TRUNC , 0664);
    //     if(wrfd < 0){
    //         perror("pic1 open");
    //         return -1;
    //     }
    //     ssize_t wrsize = write(wrfd, &tmp_buf, 1024*1024);
    //     printf("wrsize = %.3lf kB\n", wrsize/1000.0);
    //
    //
    //     //把用过的帧放回输入队列
    //     if( ioctl(fd, VIDIOC_QBUF, &tmp_buf) < 0){
    //         perror("Processing Image--VIDIOC_QBUF");
    //         return -1;
    //     }
    // }



    //将取出的帧放回队列
    if( ioctl(fd, VIDIOC_QBUF, &tmp_buf)  < 0){
        perror("Processing Image--VIDIOC_QBUF");
        return -1;
    }

    printf("Processed Image ok......\n");
    return 0;
}


/**
    停止视频采集:
    1.  停止采集; VIDIOC_STREAMOFF
    2.  释放 帧缓冲区req_buffers;(unmap)
    2.  关闭设备;
*/
int stop_catch(void)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
        perror("Closing Device--VIDIOC_STREAMOFF");
        return -1;
    }
    return 0;
}


void close_device(void)
{//关闭设备
    printf("Closing Device fd = %d...\n", fd);
    int i = 0;
    for(i; i < req_buffers.count; i++){
        munmap(NULL, frame_buf[i].length);
    }
    stop_catch();
	close(fd);
    free(frame_buf);
}




int main(int argc, char* argv[])
{
    /*1. open video0 device*/
    fd = open_file(device_fname);
    if(fd < 0){
        printf("File open failed... fd = %d\n", fd );
        exit(-1);
    }else{
        printf("File open successfully... fd = %d\n", fd );
    }

    /*2. 查看设备能力,是否有音视频输入输出?(VIDIOC_QUERYCAP：查询驱动功能)*/
    check_device_info();
    get_current_frame_fmt();

    set_catch_frame_fmt();
    get_current_frame_fmt();

    set_inout_method();

    add_to_input_queue();      //之前没有把缓冲帧加入到 输入队列

    start_catch();
    process_image();

    // // 7.3 处理缓冲数据(一帧)
    // struct v4l2_buffer buf;
    // memset(&buf, 0, sizeof(buf));
    // buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // buf.memory = V4L2_MEMORY_MMAP;
    // buf.index = 0;
    // printf("buf = %#x\n", buf);
    // printf("从缓冲区取出一帧(数据帧)\n" );
    // //从缓冲区取出一帧(数据帧)
    // if(ioctl(fd, VIDIOC_DQBUF, &buf) < 0){
    //     perror("DQBUF:");
    //     exit(-1);
    // }
    // printf("buf = %#x\n", buf);
    // //处理数据
    // int wrfd = open("pic1.jpg", O_WRONLY | O_CREAT | O_TRUNC , 0664);
    // // FILE *fp = fopen("pic2.jp", w+);
    // if(wrfd < 0){
    // // if(NULL == fp)
    //     perror("pic1 open:");
    //     exit(-1);
    // }
    // // ssize_t rdsize = fwrite(buffers[buf.index].start, buf.bytesused, 1, fp);
    // ssize_t rdsize = write(wrfd, buffers[0].start, buffers[0].length);
    // printf("read size = %d\n", rdsize);
    // printf("处理数据\n" );
    //
    // // 将取出的缓冲帧放回缓冲区, 重复利用
    // if(ioctl(fd, VIDIOC_QBUF, &buf) <0){
    //     printf("QDBUF Failed. \n");
    //     exit(-1);
    // }

    close_device();

    return 0;
}
