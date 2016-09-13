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

#define JPG_COMPRESS
#define BMP_HEADER

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

#ifdef JPG_COMPRESS
#include "setjmp.h"
#include "jpeglib.h"
#include "sys/types.h"	//size_t
#include "jerror.h"
#define JPEG_QUALITY 50 //图片质量
JSAMPLE * image_buffer;	/* Points to large array of R,G,B-order data */
int image_height = HEIGHT;	/* Number of rows in image */
int image_width = WIDTH;		/* Number of columns in image */
#endif

#define PC_DEBUG

#ifdef PC_DEBUG
static char*  device_fname = "/dev/video1";
#else
static char*  device_fname = "/dev/video0";
#endif

static char*  output_fname_yuyv = "./pic/pic1.YUV";
static char*  output_fname_rgb_bmp = "./pic/pic1.bmp";
static char*  output_fname_jpg = "./pic/pic1.jpg";

static int fd = -1; //摄像头文件描述符
static int wrfd = -1;
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

int open_file(const char* const file_name)
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

    // 这里可能有问题,sizeof(single_buff),而不是sizeof(*frame_buf)==4byte
    // frame_buf = (single_buff*) calloc (req_buffers.count, sizeof(*frame_buf));
    frame_buf = (single_buff*) calloc (req_buffers.count, sizeof(single_buff));
    if(frame_buf == NULL){
        perror("calloc:");
        exit -1;
    }else{
        printf("calloc ok! \n用户空间:frame_buf addr = %p\n",frame_buf);
        printf("用户空间:frame_buf addr = %p\n",frame_buf);
    }
    //  2.映射,把所有的缓冲帧都分别映射到用户地址空间
    unsigned int i = 0;
    struct v4l2_buffer  tmp_buf;    //驱动采集的某一帧
    printf("申请的缓冲地址:\n");
    for(i; i < req_buffers.count; i++){
        memset(&tmp_buf, 0, sizeof(tmp_buf));
        tmp_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        tmp_buf.memory = V4L2_MEMORY_MMAP;
        tmp_buf.index = i;

        //查询序号为i的缓冲区, 得到它的起始物理地址和大小,暂存到tmp_buf中
        if( -1 == ioctl(fd, VIDIOC_QUERYBUF, &tmp_buf)){
            perror("Memory Mappping--VIDIOC_QUERYBUF");
            exit -1;
        }
        // 保存长度
        frame_buf[i].length = tmp_buf.length;

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
            exit -1;
        }
        printf("buffers[%d].start = %p\n", i, frame_buf[i].start);

        //把入队放在每个申请内存后就操作,而不是单独
        if( ioctl(fd, VIDIOC_QBUF, &tmp_buf) < 0){
            perror("Adding to Input Queues");
            exit -1;
        }
    }
    printf("-----------------tmp_buf.length= %d------------\n", tmp_buf.length);

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
        exit -1;
    }
    return ret;
}


int save_to_yuyv(void)
 {//处理采集到的帧
    /**
        数据处理:
        1.  从输出队列output_queues取出存有视频的帧缓冲req_buffers[i]: VIDIOC_DQBUF
        2.  处理数据;
        3.  处理完后, 将该帧缓冲req_buffers[i] 重新放回输入队列input_queues: VIDIOC_QBUF
        start_capture(); 和 process_image(); 应放在一起实现循环采集处理
    */
    printf("save_to_yuyv.....\n");
    struct v4l2_buffer  tmp_buf;
    // memset(&tmp_buf, 0, sizeof(tmp_buf));
    tmp_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tmp_buf.memory = V4L2_MEMORY_MMAP;
    printf("---------------------------------------\n");

    if(ioctl(fd, VIDIOC_DQBUF, &tmp_buf) < 0){
        perror("VIDIOC_DQBUF");
        exit -1;
    }

    wrfd = open(output_fname_yuyv, O_RDWR | O_CREAT, 0777);
    if(wrfd < 0){
        perror("output file open");
        exit -1;
    }

    /*
    问题出现这里, 写的内容要从frame_buf[]这个数组中读取,而不是tmp_buf!!!
    */
    ssize_t wrsize = write(wrfd, frame_buf[0].start, frame_buf->length);
    printf("wrsize = %ld kB\n", wrsize/1024);

    printf("---------------------------------------\n");

    //将取出的帧放回队列
    if( ioctl(fd, VIDIOC_QBUF, &tmp_buf)  < 0){
        perror("save_to_yuyv--VIDIOC_QBUF");
        return -1;
    }

    printf("save_to_yuyv ok......\n");
    return 0;
}


int yuyv_to_rgb_pixel(int y, int u, int v)
{//根据输入的y,u,v分量,把yuyv转化为rgb格式的像素, 返回一个32bit像素的图像
    unsigned int pixel32 =  0;  //保存32bit的像素,用4byte的int保存
    unsigned char *pixel = (unsigned char*)&pixel32;    //把32bit的分成4份,用char*来指向

    int r,g,b;
    r = y + 1.4075 *(v-128);
    g = y - 0.3455 *(u-128) - 0.7169*(v-128);
    b = y + 1.779 *(u-128);
    if(r>255) r = 255;
    if(g>255) g = 255;
    if(b>255) b = 255;
    if(r<0) r = 0;
    if(g<0) g = 0;
    if(b<0) b = 0;

    pixel[0] = r;
    pixel[1] = g;
    pixel[2] = b;

    return pixel32;
}

int yuyv_to_rgb_buffer(unsigned char*yuv, unsigned char*rgb,
    unsigned int width, unsigned int height)
{//根据YUYV图片的一个像素信息yuv(4byte)和图片的width,height,放到rgb的缓存rgb中.
    unsigned int in,out = 0;
    unsigned int pixel16;
    unsigned char pixel24[3];   //3byte的像素
    unsigned int pixel32;

    int y0, u, y1, v;   //这是连续2个YUYV像素的码流状态

    //循环把2个yuyv码转化成2个rgb24的像素
    for(in = 0; in < width*height*2; in+=4){
        pixel16 =   yuv[in+3] << 24 |
                    yuv[in+2] << 16 |
                    yuv[in+1] << 8 |
                    yuv[in+0];  //把yuyv422的Y0,U0,Y1,V1先放到一个4byte的中转空间
        //然后中转空间的各个为取出,放到y,u,v
        y0 = (pixel16 & 0x000000ff);
        u = (pixel16 & 0x0000ff00) >> 8;
        y1 = (pixel16 & 0x00ff0000) >> 16;
        v = (pixel16 & 0xff000000) >> 24;

        pixel32 = yuyv_to_rgb_pixel(y0, u, v);  //生成前一个rgb像素

        pixel24[0] = (pixel32 & 0x000000ff);
        pixel24[1] = (pixel32 & 0x0000ff00) >> 8;
        pixel24[2] = (pixel32 & 0x00ff0000) >> 16;

        rgb[out++] = pixel24[0];
        rgb[out++] = pixel24[1];
        rgb[out++] = pixel24[2];

        pixel32 = yuyv_to_rgb_pixel(y1,u,v); //生成后一个rgb像素
        pixel24[0] = (pixel32 & 0x000000ff);
        pixel24[1] = (pixel32 & 0x0000ff00) >> 8;
        pixel24[2] = (pixel32 & 0x00ff0000) >> 16;
        rgb[out++] = pixel24[0];
        rgb[out++] = pixel24[1];
        rgb[out++] = pixel24[2];
    }

    return 0;
}

void set_bmp_header(bit_map_file_header *bf, bit_map_info_header *bi)
{
    //Set bit_map_info_header
    bi->biSize = 40;
    bi->biWidth = WIDTH;
    bi->biHeight = HEIGHT;
    bi->biPlanes = 1;
    bi->biBitCount = 24;
    bi->biCompression = 0;
    bi->biSizeImage = WIDTH*HEIGHT*3;
    bi->biXPelsPerMeter = 0;
    bi->biYPelsPerMeter = 0;
    bi->biClrUsed = 0;
    bi->biClrImportant = 0;
    //Set bit_map_file_header
    bf->bfType = 0x4d42;
    bf->bfSize = 54 + bi->biSizeImage;
    bf->bfReserved = 0;
    bf->bfOffBits = 54;
}

int yuyv_to_rgb24(void)
{// 把yuyv格式的图片转换成RGB24格式
    FILE* yuyv_fp =  fopen(output_fname_yuyv, "rb");
    if(yuyv_fp == NULL){
        perror("open yuyvfd");
        exit(EXIT_FAILURE);
    }
    FILE* rgb_fp =  fopen(output_fname_rgb_bmp, "wb+");
    if(rgb_fp == NULL){
        perror("open rgb_fp");
        exit(EXIT_FAILURE);
    }
    printf("yuyv_to_rgb24: open file ok.\n");

    unsigned int yuyv_size = WIDTH*HEIGHT*2;
    unsigned int rgb_size = WIDTH*HEIGHT*3;
    unsigned char yuyv_buf[yuyv_size];  //缓存YUYV数据
    unsigned char rgb_buf[rgb_size+54];    //缓存RGB数据,3中颜色+54个字节的文件头

    //读
    size_t rdsz = fread(yuyv_buf, sizeof(yuyv_buf[0]), yuyv_size, yuyv_fp);
    if(rdsz < 0){
        printf("read nothing.\n");
        return -1;
    }else{
        printf("read %ld kB.\n",rdsz/1024 );
    }

    yuyv_to_rgb_buffer(yuyv_buf, rgb_buf, WIDTH, HEIGHT);   //把yuyv转化为rgb格式

    //添加文件头
    bit_map_file_header bf;
    bit_map_info_header bi;
    set_bmp_header(&bf, &bi);


    fwrite(&bf, 14, 1, rgb_fp);
    fwrite(&bi, 40, 1, rgb_fp);
    size_t wrsz = fwrite(rgb_buf, rgb_size, 1, rgb_fp);
    printf("wrsz = %ld kB\n", (wrsz));

    fclose(yuyv_fp);
    fclose(rgb_fp);

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

    //取消映射
    for(i; i < req_buffers.count; i++){
        if(munmap(frame_buf[i].start, frame_buf[i].length)<0){
            perror("munmap");
            exit -1;
        }
    }

    stop_capture();
	close(fd);
    close(wrfd);
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


/*
 * Sample routine for JPEG compression.  We assume that the target file name
 * and a compression quality factor are passed in.
 */

GLOBAL(void)
write_JPEG_file (char* bmp_filename, char * jpg_filename, int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    int ret;
    FILE * outfile;		/* target file */
    JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
    int row_stride;		/* physical row width in image buffer */

    FILE* infile = fopen(bmp_filename, "rb");
    image_buffer = (unsigned char*)malloc(WIDTH*HEIGHT*3);
    if(infile == NULL){
        printf("ERROR1: Can not open the image.\n");
        free(image_buffer);
        exit(EXIT_FAILURE);
    }
    //    跳过bmp文件头，直接读取掌纹图像数据, 放到image_buffer
    fseek(infile, 54, SEEK_SET);
    ret = fread(image_buffer, sizeof(unsigned char)*WIDTH*HEIGHT*3, 1, infile);
    if(ret == 0)
    {
        if(ferror(infile))
        {
            printf("\nERROR2: Can not read the pixel data.\n");
            free(image_buffer);
            fclose(infile);
            exit(EXIT_FAILURE);
        }
    }


/* Step 1: allocate and initialize JPEG compression object */
    cinfo.err = jpeg_std_error(&jerr);
    //init
    jpeg_create_compress(&cinfo);

/* Step 2: specify data destination (eg, a file) */
    if ((outfile = fopen(jpg_filename, "wb")) == NULL) {
        fprintf(stderr, "can't open %s\n", jpg_filename);
        exit(-1);
    }
    jpeg_stdio_dest(&cinfo, outfile);

/* Step 3: set parameters for compression */
    cinfo.image_width = image_width; 	/* image width and height, in pixels */
    cinfo.image_height = image_height;
    cinfo.input_components = 3;	/* # of color components per pixel */
    cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */

    //set default compression parameters.
    jpeg_set_defaults(&cinfo);
    //use of quality (quantization table) scaling:
    jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);


    /* Step 4: Start compressor */
    //TRUE ensures that we will write a complete interchange-JPEG file.
    jpeg_start_compress(&cinfo, TRUE);

    /* Step 5: while (scan lines remain to be written) */
    /*           jpeg_write_scanlines(...); */
    /* Here we use the library's state variable cinfo.next_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     * To keep things simple, we pass one scanline per call; you can pass
     * more if you wish, though.
     */
    row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */
    int jpegwt = -1;
    while (cinfo.next_scanline < cinfo.image_height) {
        /* jpeg_write_scanlines expects an array of pointers to scanlines.
        * Here the array is only one element long, but you could pass
        * more than one scanline at a time if that's more convenient.
        */
        row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    /* Step 6: Finish compression */
    jpeg_finish_compress(&cinfo);
    /* After finish_compress, we can close the output file. */
    fclose(outfile);

    /* Step 7: release JPEG compression object */
    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_compress(&cinfo);


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

    /*2. 查看设备能力,是否有音视频输入输出?(VIDIOC_QUERYCAP：查询驱动功能)*/
    check_device_info();

    // /*3. 设备初始化*/
    device_init();

    /*4. 开始获取图像*/
    start_capture();

    /*5. 处理图片*/
    save_to_yuyv();
    yuyv_to_rgb24();
    write_JPEG_file(output_fname_rgb_bmp, output_fname_jpg, JPEG_QUALITY);

    /*6. 关闭设备*/
    close_device();

    return 0;
}
