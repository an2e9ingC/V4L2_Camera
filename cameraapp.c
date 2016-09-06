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
#define WIDTH 640
#define HEIGHT 480

static char*  device_fname = "/dev/video0";
static char*  output_fname = "pic1.jpg";
static int fd = -1; //摄像头文件描述符
static struct v4l2_capability cap;  //设备的属性
typedef struct _buffer  //定义一个结构体来映射每个缓冲帧
{
    void* start;    //起始地址
    unsigned int length;    //帧长度
}single_buff;

single_buff* frame_buf = NULL; //(用户空间)记录帧 映射到用户空间(包含4个用户可用的帧)

/**req_buffers: 内核视频缓冲区VIDIOC_REQBUFS,原始数据
    (3个以上, 每个存放一帧图像)
    1.  app通过VIDIOC_QUERYBUF可以查询到缓冲区在内核中的长度和偏移量地址;
    2.  用户访问缓冲区需要 地址映射 mmap() 到用户空间才能访问;
*/
struct v4l2_requestbuffers req_buffers;

int open_file(const char*const file_name)
{
    int retfd =  open(file_name, O_RDWR);
    if(retfd < 0){
       perror("File open");
       return(-1);
    }
    return retfd;
}

void  check_support_fmt(void)
{//VIDIOC_ENUM_FMT // 查询,显⽰所有⽀持的格式
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
    fmt.fmt.pix.pixelformat= V4L2_PIX_FMT_MJPEG;
    if(ioctl(fd,VIDIOC_TRY_FMT,&fmt)==-1 && errno==EINVAL){
        printf("not support format V4L2_PIX_FMT_MJPEG!\n");
    }else{
        printf("Do support format V4L2_PIX_FMT_MJPEG!\n");
    }

}


void check_device_info(void)
{//获取设备信息,支持的格式
    printf("Checking Device Info......\n");
    ioctl(fd,VIDIOC_QUERYCAP, &cap);
    printf("device name : %s\n", cap.card);
    printf("device driver : %s\n", cap.driver);
    printf("device bus_info : %s\n", cap.bus_info);
    printf("KERNEL_VERSION : %u.%u.%u ;\n", (cap.version>>16)& 0xFF,
                (cap.version>>8)&0XFF, (cap.version>>0) & 0xFF );
    printf("device capabilities : %u\n", cap.capabilities);
    if(V4L2_CAP_VIDEO_CAPTURE  & cap.capabilities){
        printf("This device DO supports iamge captureing.\n");
    }else{
        printf("This device DO NOT support iamge captureing .\n");
    }
    if(V4L2_CAP_STREAMING & cap.capabilities){
        printf("This device DO supports iamge captureing STREAMING.\n");
    }else{
        printf("This device DO NOT support iamge captureing STREAMING.\n");
    }
    check_support_fmt();
}


void get_current_frame_fmt(void)
{//查看当前视频帧格式: VIDIOC_G_FMT
    printf("Getting Current Frame Format......\n" );
    struct v4l2_format current_fmt;
    memset ( &current_fmt, 0, sizeof(current_fmt) );
    current_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ioctl(fd, VIDIOC_G_FMT, &current_fmt);
    printf("Current Frame Format: \n" );
    printf("\t size of the image = %u;\n"
            "\t width=%u;\n\t height=%u;\n",
                current_fmt.fmt.pix.sizeimage,
                current_fmt.fmt.pix.width,
                current_fmt.fmt.pix.height);
    if( current_fmt.fmt.pix.field == V4L2_FIELD_INTERLACED){
         printf("Storate format is interlaced frame format\n");
    }
    printf("Current Frame Format: %u\n", current_fmt.fmt.pix.pixelformat);


    struct v4l2_fmtdesc current_fmtdesc;
    memset ( &current_fmtdesc, 0, sizeof(current_fmtdesc) );
    current_fmtdesc.index = 0;
    current_fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while (ioctl(fd, VIDIOC_ENUM_FMT, &current_fmtdesc) != -1) {
        if( current_fmtdesc.pixelformat & current_fmt.fmt.pix.pixelformat){
            printf("\tCurrent Frame Format: %u\n", current_fmt.fmt.pix.pixelformat);
            printf("\tCurrent Frame Format: %s\n", current_fmtdesc.description);
            break;
        }
        current_fmtdesc.index++;
    }
}

int set_capture_frame_fmt(void)
{//设置捕捉的 帧格式(视频制式NTSC/PAL????)
    printf("Seting capture Frame fmt ......\n");
    struct v4l2_format fmt; //stream data format
    memset ( &fmt, 0, sizeof(fmt) );
    // definition of an image format( struct v4l2_format.fmt.pix)
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = WIDTH;
    fmt.fmt.pix.height      = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;    //定义pixel format
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

    // set fmt
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("set format");
        return -1;    // error return -1
    }
    if((fmt.fmt.pix.width == WIDTH) && (fmt.fmt.pix.height == HEIGHT)){
        printf("Seting capture Frame fmt Ok!\n");
    }else{
        printf("Seting capture Frame fmt Failed!\n");
        return -1;
    }
    return 0;   // success return 0;
}


int request_buffers(int cnt)
{//申请视频流缓冲区, 包含cnt个缓存
    printf("Setting Input/Output Method......\n");
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
        // 保存长度
        frame_buf[i].length = tmp_buf.length;
        printf("-----------------tmp_buf.length= %d------------\n", tmp_buf.length);
        printf("frame_buf[%d].length = %u\n", i, frame_buf[i].length);

        //映射到frame_buf[i].start开始地址,映射到用户空间
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
    }
}

int add_to_input_queue(void)
{//把缓冲帧 放入 驱动是视频输入队列
    printf("Adding Frame to Input Queues......\n");
    unsigned int i;

    //加入输入队列
    for(i; i < req_buffers.count; i++){
        struct v4l2_buffer  tmp_buf;
        memset(&tmp_buf, 0, sizeof(tmp_buf));
        tmp_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        tmp_buf.memory = V4L2_MEMORY_MMAP;

        tmp_buf.index = i;
        if( ioctl(fd, VIDIOC_QBUF, &tmp_buf) < 0){
            perror("Adding to Input Queues");
            return -1;
        }
    }
    return 0;
}


int start_capture(void)
{//循环采集数据
    /**
        采集数据步骤:
        1.  把缓冲区req_buffers(5个)放到视频输入队列 VIDIOC_QBUF;
            输入队列input_queues(等待驱动存放视频的队列)
        2.  启动视频获取,ioctl:VIDIOC_STREAMON;
        3.  循环采集连续视频;
        4.  将输入队列的第一个帧缓冲区 --> 输出队列
            输出队列output_queues(等待用户读取视频的队列)
    */
    printf("Starting captureing Frame......\n");

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret = -1;
    ret  = ioctl(fd, VIDIOC_STREAMON, &type);
    if(ret < 0){
        perror("Starting captureing--VIDIOC_STREAMON");
        return ret;
    }
    return ret;
}


int process_image(void)
{//处理采集到的帧
    /**
        数据处理:
        1.  从输出队列output_queues取出存有视频的帧缓冲req_buffers[i]: VIDIOC_DQBUF
        2.  处理数据;
        3.  处理完后, 将该帧缓冲req_buffers[i] 重新放回输入队列input_queues: VIDIOC_QBUF
        start_capture(); 和 process_image(); 应放在一起实现循环采集处理
    */
    printf("Processing Image......\n");
    struct v4l2_buffer  tmp_buf;
    memset(&tmp_buf, 0, sizeof(tmp_buf));
    tmp_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    printf("清空后的buffer类型:%d\n", tmp_buf.type);
    tmp_buf.memory = V4L2_MEMORY_MMAP;
    printf("---------------------------------------\n");

    ioctl(fd, VIDIOC_DQBUF, &tmp_buf);
    printf("序号:%u\n", tmp_buf.index);
    printf("buffer类型:%d\n", tmp_buf.type);
    printf("bytesused:%u\n", tmp_buf.bytesused);
    printf("flags:%u\n", tmp_buf.flags);
    printf("sequence队列序号:%u\n", tmp_buf.sequence);
    printf("缓冲帧地址 :%u\n", tmp_buf.m.offset);
    printf("缓冲帧length:%u\n", tmp_buf.length);

    int wrfd = open(output_fname, O_RDWR | O_CREAT, 0777);
    if(wrfd < 0){
        perror("output file open");
        return -1;
    }

    /*
    问题出现这里, 写的内容要从frame_buf[]这个数组中读取,而不是tmp_buf!!!
    */
    ssize_t wrsize = write(wrfd, frame_buf[0].start, frame_buf->length);
    printf("wrsize = %d kB\n", wrsize/1024);


    printf("---------------------------------------\n");

    //将取出的帧放回队列
    if( ioctl(fd, VIDIOC_QBUF, &tmp_buf)  < 0){
        perror("Processing Image--VIDIOC_QBUF");
        return -1;
    }

    printf("Processed Image ok......\n");
    return 0;
}



int stop_capture(void)
{// 停止采集
    /**
        停止视频采集:
        1.  停止采集; VIDIOC_STREAMOFF
        2.  释放 帧缓冲区req_buffers;(unmap)
        2.  关闭设备;
    */
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
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
    stop_capture();
	close(fd);
    free(frame_buf);
}

int device_init()
{//设备初始化
    /*1. 设置采集帧格式,并检测设置结果*/
    set_capture_frame_fmt();
    get_current_frame_fmt();

    /*2. 申请缓存*/
    request_buffers(REQ_BUFF_COUNT);
    /*3.获取缓冲帧的地址,长度(通过frame_buf来保存)*/
    memory_map();
}

int main(int argc, char* argv[])
{
    /*1. open video0 device*/
    fd = open_file(device_fname);
    if(fd < 0)
    {
        printf("File open failed... fd = %d\n", fd );
        exit(-1);
    }else{
        printf("File open successfully... fd = %d\n", fd );
    }

    // /*2. 查看设备能力,是否有音视频输入输出?(VIDIOC_QUERYCAP：查询驱动功能)*/
    // check_device_info();
    // get_current_frame_fmt();

    printf("xxxxxxxxxxxxxxxxxxxxxx\n" );
    // struct v4l2_input cinput;
    // memset(&cinput, 0, sizeof(cinput));

    // ioctl(fd, VIDIOC_G_INPUT, &cinput.index);//首先获得当前输入的 index,注意只是 index，要获得具体的信息，就的调用列举操作
    // ioctl (fd, VIDIOC_ENUMINPUT, &cinput);//调用列举操作，获得 input.index 对应的输入的具体信息
    // printf("Current input is '%s', supports: \n", cinput.name);
    // std.index = 0;
    // while (0==ioctl(fd, VIDIOC_ENUMINPUT, &std)) {
    //     if(std.id & cinput.std)
    //         printf("%s\n", std.name);
    //     std.index ++;
    // }
    // printf("Now input index is : %u\n", cinput.index);
    // v4l2_std_id std_id;
    // struct v4l2_standard std;
    // if(ioctl(fd, VIDIOC_G_STD, &std_id) == -1){
    //     perror("VIDIOC_G_STD");
    //     exit(-1);
    // }
    // memset(&std, 0, sizeof(std));
    // std.index = 0;
    // while( 0==ioctl(fd, VIDIOC_ENUMSTD, &std) ){
    //     if(std.id & std_id){
    //         printf("Current Standard : %s\n", std.name);
    //         break;
    //     }
    //     std.index++;
    // }
    //
    // if(errno == EINVAL || std.index == 0 ){
    //     perror("VIDIOC_ENUMSTD");
    //     return -1;
    // }
    //
    // printf("xxxxxxxxxxxxxxxxxxxxxx\n" );

    // /*3. 设备初始化*/
    device_init();

    /*4. 把缓存帧添加到缓冲队列*/
    add_to_input_queue();      //之前没有把缓冲帧加入到 输入队列

    /*4. 开始获取图像*/
    start_capture();
    /*5. 处理图片*/
    process_image();
    unsigned int i = 0xffffff;
    while(i--);
    process_image();

    /*6. 关闭设备*/
    close_device();

    return 0;
}
