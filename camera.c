#include "camera.h"


int open_camera_file(const char* const file_name)
{
    int retfd =  open(file_name, O_RDWR);
    if(retfd < 0){
       perror("camera file open");
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
    while(ioctl(camera_fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1){
        printf("\t%d.%s\n", fmtdesc.index+1, fmtdesc.description);
        fmtdesc.index++;
    }
    //检查是否支持格式V4L2_PIX_FMT_MJPEG
    struct v4l2_format fmt;
    memset ( &fmt, 0, sizeof(fmt) );
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat= V4L2_PIX_FMT_MJPEG;
    if(ioctl(camera_fd,VIDIOC_TRY_FMT,&fmt)==-1 && errno==EINVAL){
        printf("not support format V4L2_PIX_FMT_MJPEG!\n");
    }else{
        printf("Do support format V4L2_PIX_FMT_MJPEG!\n");
    }

}


void check_device_info(void)
{//获取设备信息,支持的格式
    printf("Checking Device Info......\n");
    printf("camera_fd = %d\n", camera_fd );
    ioctl(camera_fd,VIDIOC_QUERYCAP, &cap);
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

    ioctl(camera_fd, VIDIOC_G_FMT, &current_fmt);
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
    while (ioctl(camera_fd, VIDIOC_ENUM_FMT, &current_fmtdesc) != -1) {
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
    if (ioctl(camera_fd, VIDIOC_S_FMT, &fmt) < 0) {
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

    if(ioctl(camera_fd, VIDIOC_REQBUFS, &req_buffers) < 0){
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
        if( -1 == ioctl(camera_fd, VIDIOC_QUERYBUF, &tmp_buf)){
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
                camera_fd,
                tmp_buf.m.offset);
        if( MAP_FAILED == frame_buf[i].start){
            printf("Memory Mappping--mmap failed.\n");
            exit -1;
        }
        printf("buffers[%d].start = %p\n", i, frame_buf[i].start);

        //把入队放在每个申请内存后就操作,而不是单独
        if( ioctl(camera_fd, VIDIOC_QBUF, &tmp_buf) < 0){
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
    ret  = ioctl(camera_fd, VIDIOC_STREAMON, &type);
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

    if(ioctl(camera_fd, VIDIOC_DQBUF, &tmp_buf) < 0){
        perror("VIDIOC_DQBUF");
        exit -1;
    }

    yuyv_out_fd = open(output_fname_yuyv, O_RDWR | O_CREAT, 0777);
    if(yuyv_out_fd < 0){
        perror("output file open");
        exit -1;
    }

    /*
    问题出现这里, 写的内容要从frame_buf[]这个数组中读取,而不是tmp_buf!!!
    */
    ssize_t wrsize = write(yuyv_out_fd, frame_buf[0].start, frame_buf->length);
    printf("wrsize = %ld kB\n", wrsize/1024);

    printf("---------------------------------------\n");

    //将取出的帧放回队列
    if( ioctl(camera_fd, VIDIOC_QBUF, &tmp_buf)  < 0){
        perror("save_to_yuyv--VIDIOC_QBUF");
        return -1;
    }

    printf("save_to_yuyv ok......\n");
    close(yuyv_out_fd);
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
{//Set bit_map_info_header
    bi->biSize = 40;
    bi->biWidth = WIDTH;	//18th 4byte
    bi->biHeight = HEIGHT;	//22th 4byte
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
	printf("to rgb.file.\n");
    bit_map_file_header bf;
    bit_map_info_header bi;
	printf("sizeof(bf) = %u.\n", sizeof(bf));
	printf("sizeof(bi) = %u.\n", sizeof(bi));
    set_bmp_header(&bf, &bi);
	printf("sizeof(bf) = %u.\n", sizeof(bf));
	printf("sizeof(bi) = %u.\n", sizeof(bi));


    fwrite(&bf, 14, 1, rgb_fp);
    fwrite(&bi, 40, 1, rgb_fp);
    size_t wrsz = fwrite(rgb_buf, rgb_size, 1, rgb_fp);
    printf("wrsz = %ld kB\n", (wrsz));

    fclose(yuyv_fp);
    fclose(rgb_fp);

}

int rgb24_to_shm(void* shm_addr)
{
    printf("---------------in rgb24_to_shm----------\n");
    FILE* yuyv_fp =  fopen(output_fname_yuyv, "rb");
    if(yuyv_fp == NULL){
        perror("open yuyvfd");
        exit(EXIT_FAILURE);
    }

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
	printf("sizeof(bf) = %u.\n", sizeof(bf));
	printf("sizeof(bi) = %u.\n", sizeof(bi));
    set_bmp_header(&bf, &bi);
	printf("sizeof(bf) = %d.\n", sizeof(bf));
	printf("sizeof(bi) = %d.\n", sizeof(bi));
	

	//把头加入到共享内存 ,,  注意在 
    memcpy(shm_addr, &bf, 14);
    memcpy(shm_addr+14, &bi, 40);

	// 吧数据加入
    int i = sizeof(rgb_buf);
    printf("rgb_buf size = %d Byte \n", i);
    if( memcpy(shm_addr+54, rgb_buf, i) < 0){
        err_exit("strncat rgb_buf");
    }
    printf("---------------strcat2 ok----------\n");

    FILE* rgb_fp =  fopen(output_fname_rgb_bmp, "wb+");
    if(rgb_fp == NULL){
        perror("open rgb_fp");
        exit(EXIT_FAILURE);
    }
    printf("rgb24_to_bmpfile : open file ok.\n");
	char bmp_buf[WIDTH*HEIGHT*3+55] ={0	};
	memcpy(bmp_buf, shm_addr, WIDTH*HEIGHT*3+54);
	size_t fw =	fwrite(bmp_buf, sizeof(bmp_buf), 1, rgb_fp);
	if(fw < 0){
		printf("fw = %d\n", fw);
		err_exit("fwrite");
	}


    if( shmdt(shm_addr) < 0){
        err_exit("shmdt");
    }
    printf("---------------shmdt ok----------\n");

    fclose(yuyv_fp);
    printf("---------------out rgb24_to_shm----------\n");

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
    if(ioctl(camera_fd, VIDIOC_STREAMOFF, &type) < 0){
        perror("Closing Device--VIDIOC_STREAMOFF");
        return -1;
    }
    return 0;
}

void close_camera(void)
{//关闭设备
    printf("Closing Device camera_fd = %d...\n", camera_fd);
    int i = 0;

    //取消映射
    for(i; i < req_buffers.count; i++){
        if(munmap(frame_buf[i].start, frame_buf[i].length)<0){
            perror("munmap");
            exit -1;
        }
    }

    stop_capture();
	close(camera_fd);
    close(yuyv_out_fd);
    free(frame_buf);
}

int device_init(void)
{//设备初始化
    /*1. 设置采集帧格式,并检测设置结果*/
    if(set_capture_frame_fmt() < 0){
        exit(EXIT_FAILURE);
    }
    get_current_frame_fmt();

    /*2. 申请缓存*/
    if(request_buffers(REQ_BUFF_COUNT) < 0){
        exit(EXIT_FAILURE);
    }
    /*3.获取缓冲帧的地址,长度(通过frame_buf来保存)*/
    if(memory_map() <0 ){
        exit(EXIT_FAILURE);
    }
}

/*
 * Sample routine for JPEG compression.  We assume that the target file name
 * and a compression quality factor are passed in.
 */
void write_JPEG_file (char* bmp_filename, char * jpg_filename, int quality)
{//根据bmp文件和quality, 生成jpg文件jpg_filename
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
    cinfo.image_width = WIDTH; 	/* image width and height, in pixels */
    cinfo.image_height = HEIGHT;
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
    row_stride = WIDTH * 3;	/* JSAMPLEs per row in image_buffer */
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
