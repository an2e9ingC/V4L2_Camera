diff --git a/Makefile b/Makefile
index 9f3db54..5500d1d 100644
--- a/Makefile
+++ b/Makefile
@@ -1,8 +1,9 @@
 
 all:
+	gcc cameraapp.c -o a.out
 	arm-none-linux-gnueabi-gcc cameraapp.c -o camera.app
 	sudo cp -f camera.app  /home/michael/mynfs/rootfs/workspace/camera
 clean:
 	rm -rf *.o *~ core .depend  *.ko *.mod.c .tmp_versions/ Module* modules*
 	sudo rm -f  /home/michael/mynfs/rootfs/workspace/camera/camera.app
-	rm -f camera.app
+	rm -f camera.app a.out
diff --git a/camera.app b/camera.app
index 0493d0c..953a5a3 100755
Binary files a/camera.app and b/camera.app differ
diff --git a/cameraapp.c b/cameraapp.c
index 790c379..9f4b53d 100644
--- a/cameraapp.c
+++ b/cameraapp.c
@@ -11,30 +11,31 @@
 #include <sys/mman.h>
 
 #define REQ_BUFF_COUNT 5
+#define WIDTH 640
+#define HEIGHT 480
 
-const char* const device_fname = "/dev/video0";
-const char* const output_fname = "catch1";
+static char*  device_fname = "/dev/video0";
+static char*  output_fname = "pic1.jpg";
 static int fd = -1; //摄像头文件描述符
+static struct v4l2_capability cap;  //设备的属性
 typedef struct _buffer  //定义一个结构体来映射每个缓冲帧
 {
     void* start;    //起始地址
     unsigned int length;    //帧长度
 }single_buff;
+
 single_buff* frame_buf = NULL; //(用户空间)记录帧 映射到用户空间(包含4个用户可用的帧)
 
-/**
-    req_buffers: 内核视频缓冲区VIDIOC_REQBUFS,原始数据
+/**req_buffers: 内核视频缓冲区VIDIOC_REQBUFS,原始数据
     (3个以上, 每个存放一帧图像)
     1.  app通过VIDIOC_QUERYBUF可以查询到缓冲区在内核中的长度和偏移量地址;
     2.  用户访问缓冲区需要 地址映射 mmap() 到用户空间才能访问;
 */
 struct v4l2_requestbuffers req_buffers;
 
-void  check_support_fmt(void);
-
 int open_file(const char*const file_name)
 {
-    int retfd =  open(file_name, O_RDWR,  0);
+    int retfd =  open(file_name, O_RDWR);
     if(retfd < 0){
        perror("File open");
        return(-1);
@@ -42,27 +43,8 @@ int open_file(const char*const file_name)
     return retfd;
 }
 
-void check_device_info(void)
-{//获取设备信息,支持的格式
-    printf("Checking Device Info......\n");
-    struct v4l2_capability cap;
-    ioctl(fd,VIDIOC_QUERYCAP, &cap);
-    printf("device name : %s\n", cap.card);
-    printf("device driver : %s\n", cap.driver);
-    printf("device bus_info : %s\n", cap.bus_info);
-    printf("KERNEL_VERSION : %u.%u.%u ;\n", (cap.version>>16)& 0xFF,
-                (cap.version>>8)&0XFF, (cap.version>>0) & 0xFF );
-    printf("device capabilities : %u\n", cap.capabilities);
-    if(V4L2_BUF_TYPE_VIDEO_CAPTURE == cap.capabilities){
-        printf("This device supports iamge.\n");
-    }else{
-        printf("This device does not support iamge.\n");
-    }
-    check_support_fmt();
-}
-
 void  check_support_fmt(void)
-{//VIDIOC_ENUM_FMT // 显⽰所有⽀持的格式
+{//VIDIOC_ENUM_FMT // 查询,显⽰所有⽀持的格式
     printf("Checking Device Supported Format......\n");
     struct v4l2_fmtdesc fmtdesc;
     fmtdesc.index = 0;
@@ -76,7 +58,7 @@ void  check_support_fmt(void)
     struct v4l2_format fmt;
     memset ( &fmt, 0, sizeof(fmt) );
     fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
-    fmt.fmt.pix.pixelformat=V4L2_PIX_FMT_MJPEG;
+    fmt.fmt.pix.pixelformat= V4L2_PIX_FMT_MJPEG;
     if(ioctl(fd,VIDIOC_TRY_FMT,&fmt)==-1 && errno==EINVAL){
         printf("not support format V4L2_PIX_FMT_MJPEG!\n");
     }else{
@@ -85,6 +67,31 @@ void  check_support_fmt(void)
 
 }
 
+
+void check_device_info(void)
+{//获取设备信息,支持的格式
+    printf("Checking Device Info......\n");
+    ioctl(fd,VIDIOC_QUERYCAP, &cap);
+    printf("device name : %s\n", cap.card);
+    printf("device driver : %s\n", cap.driver);
+    printf("device bus_info : %s\n", cap.bus_info);
+    printf("KERNEL_VERSION : %u.%u.%u ;\n", (cap.version>>16)& 0xFF,
+                (cap.version>>8)&0XFF, (cap.version>>0) & 0xFF );
+    printf("device capabilities : %u\n", cap.capabilities);
+    if(V4L2_CAP_VIDEO_CAPTURE  & cap.capabilities){
+        printf("This device DO supports iamge captureing.\n");
+    }else{
+        printf("This device DO NOT support iamge captureing .\n");
+    }
+    if(V4L2_CAP_STREAMING & cap.capabilities){
+        printf("This device DO supports iamge captureing STREAMING.\n");
+    }else{
+        printf("This device DO NOT support iamge captureing STREAMING.\n");
+    }
+    check_support_fmt();
+}
+
+
 void get_current_frame_fmt(void)
 {//查看当前视频帧格式: VIDIOC_G_FMT
     printf("Getting Current Frame Format......\n" );
@@ -94,7 +101,7 @@ void get_current_frame_fmt(void)
 
     ioctl(fd, VIDIOC_G_FMT, &current_fmt);
     printf("Current Frame Format: \n" );
-    printf("\t size of the buffer = %d;\n"
+    printf("\t size of the image = %u;\n"
             "\t width=%u;\n\t height=%u;\n",
                 current_fmt.fmt.pix.sizeimage,
                 current_fmt.fmt.pix.width,
@@ -102,32 +109,33 @@ void get_current_frame_fmt(void)
     if( current_fmt.fmt.pix.field == V4L2_FIELD_INTERLACED){
          printf("Storate format is interlaced frame format\n");
     }
-    printf("Current Format: %ul\n", current_fmt.fmt.pix.pixelformat);
-
-    // struct v4l2_fmtdesc current_fmtdesc;
-    // memset ( &current_fmtdesc, 0, sizeof(current_fmtdesc) );
-    // current_fmtdesc = current_fmt.fmt.pix.pixelformat;
-    // current_fmtdesc.index = 0;
-    // current_fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
-    // while (ioctl(fd, VIDIOC_G_FMT, &current_fmtdesc) != -1) {
-    //     if( current_fmtdesc.pixelformat & current_fmt.fmt.pix.pixelformat){
-    //         printf("Current Format: %s\n", current_fmtdesc.description);
-    //     }
-    //     current_fmtdesc.index++;
-    // }
+    printf("Current Frame Format: %u\n", current_fmt.fmt.pix.pixelformat);
+
+
+    struct v4l2_fmtdesc current_fmtdesc;
+    memset ( &current_fmtdesc, 0, sizeof(current_fmtdesc) );
+    current_fmtdesc.index = 0;
+    current_fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
+    while (ioctl(fd, VIDIOC_ENUM_FMT, &current_fmtdesc) != -1) {
+        if( current_fmtdesc.pixelformat & current_fmt.fmt.pix.pixelformat){
+            printf("\tCurrent Frame Format: %u\n", current_fmt.fmt.pix.pixelformat);
+            printf("\tCurrent Frame Format: %s\n", current_fmtdesc.description);
+            break;
+        }
+        current_fmtdesc.index++;
+    }
 }
 
-int set_catch_frame_fmt(void)
-{//设置捕捉的 帧格式
-    printf("Seting Catch Frame fmt ......\n");
+int set_capture_frame_fmt(void)
+{//设置捕捉的 帧格式(视频制式NTSC/PAL????)
+    printf("Seting capture Frame fmt ......\n");
     struct v4l2_format fmt; //stream data format
     memset ( &fmt, 0, sizeof(fmt) );
     // definition of an image format( struct v4l2_format.fmt.pix)
     fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
-    fmt.fmt.pix.width       = 640;
-    fmt.fmt.pix.height      = 480;
-    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
-    // fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
+    fmt.fmt.pix.width       = WIDTH;
+    fmt.fmt.pix.height      = HEIGHT;
+    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;    //定义pixel format
     fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
 
     // set fmt
@@ -135,22 +143,29 @@ int set_catch_frame_fmt(void)
         perror("set format");
         return -1;    // error return -1
     }
-    printf("Seting Catch Frame fmt Ok!\n");
+    if((fmt.fmt.pix.width == WIDTH) && (fmt.fmt.pix.height == HEIGHT)){
+        printf("Seting capture Frame fmt Ok!\n");
+    }else{
+        printf("Seting capture Frame fmt Failed!\n");
+        return -1;
+    }
     return 0;   // success return 0;
 }
 
 
 int request_buffers(int cnt)
 {//申请视频流缓冲区, 包含cnt个缓存
+    printf("Setting Input/Output Method......\n");
     memset(&req_buffers, 0, sizeof(req_buffers));
-
     req_buffers.count = cnt;
     req_buffers.type =  V4L2_BUF_TYPE_VIDEO_CAPTURE;
     req_buffers.memory = V4L2_MEMORY_MMAP;
+
     if(ioctl(fd, VIDIOC_REQBUFS, &req_buffers) < 0){
         perror("request_buffers: VIDIOC_REQBUFS");
         return -1;
     }
+
     if (req_buffers.count < 2){
         printf("insufficient buffer memory\n");
         printf("Number of buffers allocated = %d\n", req_buffers.count);
@@ -185,10 +200,12 @@ int memory_map(void)
             perror("Memory Mappping--VIDIOC_QUERYBUF");
             return -1;
         }
-        //映射到frame_buf[i].start开始地址,映射到用户空间
+        // 保存长度
         frame_buf[i].length = tmp_buf.length;
+        printf("-----------------tmp_buf.length= %d------------\n", tmp_buf.length);
         printf("frame_buf[%d].length = %u\n", i, frame_buf[i].length);
 
+        //映射到frame_buf[i].start开始地址,映射到用户空间
         frame_buf[i].start =
             mmap(NULL,
                 tmp_buf.length,
@@ -201,33 +218,20 @@ int memory_map(void)
             return -1;
         }
         printf("buffers[%d].start = %#x\n", i, frame_buf[i].start);
-        // 保存长度
-
     }
 }
 
-
-int set_inout_method(void)
-{//申请管理缓冲区
-    //设置输入输出方法(缓冲区管理): 使用内存映射mmap
-    printf("Setting Input/Output Method......\n");
-    //1.申请缓冲区(包含REQ_BUFF_COUNT个帧缓冲)
-    request_buffers(REQ_BUFF_COUNT);
-    //2.获取缓冲帧的地址,长度(通过frame_buf来保存)
-    memory_map();
-}
-
 int add_to_input_queue(void)
 {//把缓冲帧 放入 驱动是视频输入队列
     printf("Adding Frame to Input Queues......\n");
     unsigned int i;
-    struct v4l2_buffer  tmp_buf;
-    memset(&tmp_buf, 0, sizeof(tmp_buf));
-    tmp_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
-    tmp_buf.memory = V4L2_MEMORY_MMAP;
+
     //加入输入队列
     for(i; i < req_buffers.count; i++){
-
+        struct v4l2_buffer  tmp_buf;
+        memset(&tmp_buf, 0, sizeof(tmp_buf));
+        tmp_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
+        tmp_buf.memory = V4L2_MEMORY_MMAP;
 
         tmp_buf.index = i;
         if( ioctl(fd, VIDIOC_QBUF, &tmp_buf) < 0){
@@ -238,45 +242,47 @@ int add_to_input_queue(void)
     return 0;
 }
 
-/**
-    采集数据步骤:
-    1.  把缓冲区req_buffers(5个)放到视频输入队列 VIDIOC_QBUF;
-        输入队列input_queues(等待驱动存放视频的队列)
-    2.  启动视频获取,ioctl:VIDIOC_STREAMON;
-    3.  循环采集连续视频;
-    4.  将输入队列的第一个帧缓冲区 --> 输出队列
-        输出队列output_queues(等待用户读取视频的队列)
-*/
-int start_catch(void)
+
+int start_capture(void)
 {//循环采集数据
-    printf("Starting Catching Frame......\n");
+    /**
+        采集数据步骤:
+        1.  把缓冲区req_buffers(5个)放到视频输入队列 VIDIOC_QBUF;
+            输入队列input_queues(等待驱动存放视频的队列)
+        2.  启动视频获取,ioctl:VIDIOC_STREAMON;
+        3.  循环采集连续视频;
+        4.  将输入队列的第一个帧缓冲区 --> 输出队列
+            输出队列output_queues(等待用户读取视频的队列)
+    */
+    printf("Starting captureing Frame......\n");
 
     enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
-    int ret = 0;
+    int ret = -1;
     ret  = ioctl(fd, VIDIOC_STREAMON, &type);
     if(ret < 0){
-        perror("Starting Catching--VIDIOC_STREAMON");
+        perror("Starting captureing--VIDIOC_STREAMON");
         return ret;
     }
     return ret;
 }
 
-/**
-    数据处理:
-    1.  从输出队列output_queues取出存有视频的帧缓冲req_buffers[i]: VIDIOC_DQBUF
-    2.  处理数据;
-    3.  处理完后, 将该帧缓冲req_buffers[i] 重新放回输入队列input_queues: VIDIOC_QBUF
-    start_catch(); 和 process_image(); 应放在一起实现循环采集处理
-*/
+
 int process_image(void)
 {//处理采集到的帧
+    /**
+        数据处理:
+        1.  从输出队列output_queues取出存有视频的帧缓冲req_buffers[i]: VIDIOC_DQBUF
+        2.  处理数据;
+        3.  处理完后, 将该帧缓冲req_buffers[i] 重新放回输入队列input_queues: VIDIOC_QBUF
+        start_capture(); 和 process_image(); 应放在一起实现循环采集处理
+    */
     printf("Processing Image......\n");
     struct v4l2_buffer  tmp_buf;
     memset(&tmp_buf, 0, sizeof(tmp_buf));
     tmp_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     printf("清空后的buffer类型:%d\n", tmp_buf.type);
     tmp_buf.memory = V4L2_MEMORY_MMAP;
-    printf("xxxxxxxxxxxxx......\n");
+    printf("---------------------------------------\n");
 
     ioctl(fd, VIDIOC_DQBUF, &tmp_buf);
     printf("序号:%u\n", tmp_buf.index);
@@ -287,38 +293,21 @@ int process_image(void)
     printf("缓冲帧地址 :%u\n", tmp_buf.m.offset);
     printf("缓冲帧length:%u\n", tmp_buf.length);
 
-    int wrfd = open(output_fname, O_WRONLY | O_CREAT | O_TRUNC , 0664);
+    int wrfd = open(output_fname, O_RDWR | O_CREAT, 0777);
     if(wrfd < 0){
         perror("output file open");
         return -1;
     }
-    ssize_t wrsize = write(wrfd, &tmp_buf, tmp_buf.bytesused);
-    printf("wrsize = %.3lf kB\n", wrsize);
-
-    // int i = 4;
-    // while(i--){
-    //     ioctl(fd, VIDIOC_DQBUF, &tmp_buf);
-    //     ioctl(fd, VIDIOC_QUERYBUF, &tmp_buf);
-    //     printf("tmp_buf.bytesused = %u Byte\n", tmp_buf.bytesused);
-    //     // //处理数据
-    //     int wrfd = open("pic1.jpg", O_WRONLY | O_CREAT | O_TRUNC , 0664);
-    //     if(wrfd < 0){
-    //         perror("pic1 open");
-    //         return -1;
-    //     }
-    //     ssize_t wrsize = write(wrfd, &tmp_buf, 1024*1024);
-    //     printf("wrsize = %.3lf kB\n", wrsize/1000.0);
-    //
-    //
-    //     //把用过的帧放回输入队列
-    //     if( ioctl(fd, VIDIOC_QBUF, &tmp_buf) < 0){
-    //         perror("Processing Image--VIDIOC_QBUF");
-    //         return -1;
-    //     }
-    // }
 
+    /*
+    问题出现这里, 写的内容要从frame_buf[]这个数组中读取,而不是tmp_buf!!!
+    */
+    ssize_t wrsize = write(wrfd, frame_buf[0].start, frame_buf->length);
+    printf("wrsize = %d kB\n", wrsize/1024);
 
 
+    printf("---------------------------------------\n");
+
     //将取出的帧放回队列
     if( ioctl(fd, VIDIOC_QBUF, &tmp_buf)  < 0){
         perror("Processing Image--VIDIOC_QBUF");
@@ -330,16 +319,17 @@ int process_image(void)
 }
 
 
-/**
-    停止视频采集:
-    1.  停止采集; VIDIOC_STREAMOFF
-    2.  释放 帧缓冲区req_buffers;(unmap)
-    2.  关闭设备;
-*/
-int stop_catch(void)
-{
+
+int stop_capture(void)
+{// 停止采集
+    /**
+        停止视频采集:
+        1.  停止采集; VIDIOC_STREAMOFF
+        2.  释放 帧缓冲区req_buffers;(unmap)
+        2.  关闭设备;
+    */
     enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
-    if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
+    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
         perror("Closing Device--VIDIOC_STREAMOFF");
         return -1;
     }
@@ -354,72 +344,91 @@ void close_device(void)
     for(i; i < req_buffers.count; i++){
         munmap(NULL, frame_buf[i].length);
     }
-    stop_catch();
+    stop_capture();
 	close(fd);
     free(frame_buf);
 }
 
+int device_init()
+{//设备初始化
+    /*1. 设置采集帧格式,并检测设置结果*/
+    set_capture_frame_fmt();
+    get_current_frame_fmt();
 
-
+    /*2. 申请缓存*/
+    request_buffers(REQ_BUFF_COUNT);
+    /*3.获取缓冲帧的地址,长度(通过frame_buf来保存)*/
+    memory_map();
+}
 
 int main(int argc, char* argv[])
 {
     /*1. open video0 device*/
     fd = open_file(device_fname);
-    if(fd < 0){
+    if(fd < 0)
+    {
         printf("File open failed... fd = %d\n", fd );
         exit(-1);
     }else{
         printf("File open successfully... fd = %d\n", fd );
     }
 
-    /*2. 查看设备能力,是否有音视频输入输出?(VIDIOC_QUERYCAP：查询驱动功能)*/
-    check_device_info();
-    get_current_frame_fmt();
-
-    set_catch_frame_fmt();
-    get_current_frame_fmt();
-
-    set_inout_method();
-
-    add_to_input_queue();      //之前没有把缓冲帧加入到 输入队列
-
-    start_catch();
-    process_image();
-
-    // // 7.3 处理缓冲数据(一帧)
-    // struct v4l2_buffer buf;
-    // memset(&buf, 0, sizeof(buf));
-    // buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
-    // buf.memory = V4L2_MEMORY_MMAP;
-    // buf.index = 0;
-    // printf("buf = %#x\n", buf);
-    // printf("从缓冲区取出一帧(数据帧)\n" );
-    // //从缓冲区取出一帧(数据帧)
-    // if(ioctl(fd, VIDIOC_DQBUF, &buf) < 0){
-    //     perror("DQBUF:");
-    //     exit(-1);
+    // /*2. 查看设备能力,是否有音视频输入输出?(VIDIOC_QUERYCAP：查询驱动功能)*/
+    // check_device_info();
+    // get_current_frame_fmt();
+
+    printf("xxxxxxxxxxxxxxxxxxxxxx\n" );
+    // struct v4l2_input cinput;
+    // memset(&cinput, 0, sizeof(cinput));
+
+    // ioctl(fd, VIDIOC_G_INPUT, &cinput.index);//首先获得当前输入的 index,注意只是 index，要获得具体的信息，就的调用列举操作
+    // ioctl (fd, VIDIOC_ENUMINPUT, &cinput);//调用列举操作，获得 input.index 对应的输入的具体信息
+    // printf("Current input is '%s', supports: \n", cinput.name);
+    // std.index = 0;
+    // while (0==ioctl(fd, VIDIOC_ENUMINPUT, &std)) {
+    //     if(std.id & cinput.std)
+    //         printf("%s\n", std.name);
+    //     std.index ++;
     // }
-    // printf("buf = %#x\n", buf);
-    // //处理数据
-    // int wrfd = open("pic1.jpg", O_WRONLY | O_CREAT | O_TRUNC , 0664);
-    // // FILE *fp = fopen("pic2.jp", w+);
-    // if(wrfd < 0){
-    // // if(NULL == fp)
-    //     perror("pic1 open:");
+    // printf("Now input index is : %u\n", cinput.index);
+    // v4l2_std_id std_id;
+    // struct v4l2_standard std;
+    // if(ioctl(fd, VIDIOC_G_STD, &std_id) == -1){
+    //     perror("VIDIOC_G_STD");
     //     exit(-1);
     // }
-    // // ssize_t rdsize = fwrite(buffers[buf.index].start, buf.bytesused, 1, fp);
-    // ssize_t rdsize = write(wrfd, buffers[0].start, buffers[0].length);
-    // printf("read size = %d\n", rdsize);
-    // printf("处理数据\n" );
+    // memset(&std, 0, sizeof(std));
+    // std.index = 0;
+    // while( 0==ioctl(fd, VIDIOC_ENUMSTD, &std) ){
+    //     if(std.id & std_id){
+    //         printf("Current Standard : %s\n", std.name);
+    //         break;
+    //     }
+    //     std.index++;
+    // }
     //
-    // // 将取出的缓冲帧放回缓冲区, 重复利用
-    // if(ioctl(fd, VIDIOC_QBUF, &buf) <0){
-    //     printf("QDBUF Failed. \n");
-    //     exit(-1);
+    // if(errno == EINVAL || std.index == 0 ){
+    //     perror("VIDIOC_ENUMSTD");
+    //     return -1;
     // }
+    //
+    // printf("xxxxxxxxxxxxxxxxxxxxxx\n" );
+
+    // /*3. 设备初始化*/
+    device_init();
+
+    /*4. 把缓存帧添加到缓冲队列*/
+    add_to_input_queue();      //之前没有把缓冲帧加入到 输入队列
+
+    /*4. 开始获取图像*/
+    start_capture();
+    /*5. 处理图片*/
+    process_image();
+    unsigned int i = 0xffffff;
+    while(i--);
+    process_image();
 
+    /*6. 关闭设备*/
     close_device();
 
     return 0;
diff --git a/cp.sh b/cp.sh
deleted file mode 100755
index fdbaea3..0000000
--- a/cp.sh
+++ /dev/null
@@ -1,9 +0,0 @@
-#########################################################################
-# File Name: cp.sh
-# Author: Michael Jay
-# mail: michaelxu2016@yahoo.com
-# Created Time: 2016年08月16日 星期二 16时22分02秒
-# Function: 
-#########################################################################
-#!/bin/bash
-	 cp *.ko  ~/mynfs/rootfs/kotest/ -f
diff --git a/log/v1.png b/log/v1.png
index 7dbf547..5dcfeb1 100644
Binary files a/log/v1.png and b/log/v1.png differ
diff --git a/tmp.c b/tmp.c
deleted file mode 100644
index 839f561..0000000
--- a/tmp.c
+++ /dev/null
@@ -1,373 +0,0 @@
-/*
-*
-VIDIOC_REQBUFS：分配内存
-VIDIOC_QUERYBUF：把VIDIOC_REQBUFS中分配的数据缓存转换成物理地址
-VIDIOC_QUERYCAP：查询驱动功能
-VIDIOC_ENUM_FMT：获取当前驱动支持的视频格式
-VIDIOC_S_FMT：设置当前驱动的频捕获格式
-VIDIOC_G_FMT：读取当前驱动的频捕获格式
-VIDIOC_TRY_FMT：验证当前驱动的显示格式
-VIDIOC_CROPCAP：查询驱动的修剪能力,即缩放
-VIDIOC_S_CROP：设置视频信号的边框
-VIDIOC_G_CROP：读取视频信号的边框
-VIDIOC_QBUF：把数据从缓存中读取出来
-VIDIOC_DQBUF：把数据放回缓存队列
-VIDIOC_STREAMON：开始视频显示 函数
-VIDIOC_STREAMOFF：结束视频显示 函数
-VIDIOC_QUERYSTD：检查当前视频设备支持的标准，例如PAL或NTSC。
-*/
--------------------------------------------------------------------
-struct v4l2_crop {
-	__u32			type;	/* enum v4l2_buf_type */
-	struct v4l2_rect        c;
-};
-struct v4l2_rect {
-	__s32   left;
-	__s32   top;
-	__u32   width;
-	__u32   height;
-};
-
-设置采集窗口就是在摄像头设备的取景范围之内设定⼀个视频采集区域。主要是对结构体v4l2_crop赋
-值，v4l2_crop由⼀个v4l2_buffer_type枚举类型的type和v4l2_rect类型的结构体c构成，
-来描述视频采集窗口的类型和⼤⼩。type设置为视频采集类型V4L2_BUF_TYPE_VIDEO_CAPTURE。
-c是表⽰采集窗口的⼤⼩的结构体(左右上下)，它的成员Left和Top分别表⽰视频采集区域的起始横坐标
-和纵坐标，width和height分别表⽰采集图像的宽度和⾼度。赋值后，⽤ioctl函数通过这个结构体对
-fd进⾏设置。
--------------------------------------------------------
-/** check_device_func();
-  * struct v4l2_capability - Describes V4L2 device caps returned by VIDIOC_QUERYCAP
-  *
-  * @driver:	   name of the driver module (e.g. "bttv")
-  * @card:	   name of the card (e.g. "Hauppauge WinTV")
-  * @bus_info:	   name of the bus (e.g. "PCI:" + pci_name(pci_dev) )
-  * @version:	   KERNEL_VERSION
-  * @capabilities: capabilities of the physical device as a whole
-  * @device_caps:  capabilities accessed via this particular device (node)
-  * @reserved:	   reserved fields for future extensions
-  */
-struct v4l2_capability {
-	__u8	driver[16];
-	__u8	card[32];
-	__u8	bus_info[32];
-	__u32   version;
-	__u32	capabilities;
-	__u32	device_caps;
-	__u32	reserved[3];
-};
-
-------------------------------------------------------
-获取当前支持的格式
-int ioctl(int fd, int request, struct v4l2_fmtdesc *argp);
-struct v4l2_fmtdesc
-{
-    __u32 index; // 要查询的格式序号，应⽤程序设置
-    enum v4l2_buf_type type; // 帧类型，应⽤程序设置
-    __u32 flags; // 是否为压缩格式
-    __u8 description[32]; // 格式名称
-    __u32 pixelformat; // 格式
-    __u32 reserved[4]; // 保留
-};
-_____________________________________________________
-// 查看或设置当前格式
-VIDIOC_G_FMT, VIDIOC_S_FMT
-// 检查是否⽀持某种格式
-VIDIOC_TRY_FMT
-int ioctl(int fd, int request, struct v4l2_format *argp);
-struct v4l2_format
-{
-    enum v4l2_buf_type type;// 帧类型，应⽤程序设置
-    union fmt
-    {
-        struct v4l2_pix_format pix;// 视频设备使⽤
-        struct v4l2_window win;
-        struct v4l2_vbi_format vbi;
-        struct v4l2_sliced_vbi_format sliced;
-        __u8 raw_data[200];
-    };
-};
-struct v4l2_pix_format
-{
-    __u32 width; // 帧宽，单位像素
-    __u32 height; // 帧⾼，单位像素
-    __u32 pixelformat; // 帧格式
-    enum  v4l2_field field;
-    __u32 bytesperline;
-    __u32 sizeimage;
-    enum  v4l2_colorspace colorspace;
-    __u32 priv;
-};
-_______________________________________________________
-图像缩放
-VIDIOC_CROPCAP
-int ioctl(int fd, int request, struct v4l2_cropcap *argp);
-struct v4l2_cropcap
-{
-    enum v4l2_buf_type type;// 应⽤程序设置
-    struct v4l2_rect bounds;// 最⼤边界
-    struct v4l2_rect defrect;// 默认值
-    struct v4l2_fract pixelaspect;
-}
-// 设置缩放
-VIDIOC_G_CROP:读取视频信号的边框
-VIDIOC_S_CROP:设置视频信号的边框
-int ioctl(int fd, int request, struct v4l2_crop *argp);
-int ioctl(int fd, int request, const struct v4l2_crop *argp);
-struct v4l2_crop
-{
-    enum v4l2_buf_type type;// 应⽤程序设置
-    struct v4l2_rect c;
-}
-_______________________________________________________
-
-缓冲区申请管理:
-// 向设备申请缓冲区
-VIDIOC_REQBUFS
-int ioctl(int fd, int request, struct v4l2_requestbuffers *argp);
-struct v4l2_requestbuffers
-{
-    __u32 count; // 缓冲区内缓冲帧的数⽬
-    enum v4l2_buf_type type; // 缓冲帧数据格式
-    enum v4l2_memory memory; // 区别是内存映射还是⽤⼾指针⽅式
-    __u32 reserved[2];
-};
-enum v4l2_memoy {V4L2_MEMORY_MMAP,V4L2_MEMORY_USERPTR};
-//count,type,memory 都要应⽤程序设
-
-//获取缓冲区地址,长度
-获取缓冲帧的地址，长度：
-VIDIOC_QUERYBUF
-int ioctl(int fd, int request, struct v4l2_buffer *argp);
-struct v4l2_buffer
-{
-    __u32 index; //buffer 序号
-    enum v4l2_buf_type type; //buffer 类型
-    __u32 byteused; //buffer 中已使⽤的字节数
-    __u32 flags; // 区分是MMAP 还是USERPTR
-    enum v4l2_field field;
-    struct timeval timestamp;// 获取第⼀个字节时的系统时间
-    struct v4l2_timecode timecode;
-    __u32 sequence; // 队列中的序号
-    enum v4l2_memory memory;//IO ⽅式，被应⽤程序设置
-    union m
-    {
-        __u32 offset;// 缓冲帧地址，只对MMAP 有效
-        unsigned long userptr;
-    };
-    __u32 length;// 缓冲帧长度
-    __u32 input;
-    __u32 reserved;
-};
-//MMAP ，定义⼀个结构体来映射每个缓冲帧。
-Struct buffer
-{
-    void* start;
-    unsigned int length;
-}*buffers;
-
-void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t
-offset);
-//addr 映射起始地址，⼀般为NULL ，让内核⾃动选择
-//length 被映射内存块的长度
-//prot 标志映射后能否被读写，其值为PROT_EXEC,PROT_READ,PROT_WRITE, PROT_NONE
-//flags 确定此内存映射能否被其他进程共享，MAP_SHARED,MAP_PRIVATE
-//offset, 确定被映射的内存地址
-返回成功映射后的地址，不成功返回MAP_FAILED ((void*)-1);
-int munmap(void *addr, size_t length);// 断开映射
-//addr 为映射后的地址，length 为映射后的内存长度
-
-
-_______________________________________________________
-
-
-
-
-
-/**
-    设置视频点阵格式和点阵⼤⼩
-主要是对结构体v4l2_format进⾏赋值，它由type和联合体fmt构成，来描述视频设备当前⾏为和数据的格式。
-type--赋值为视频采集类型V4L2_BUF_TYPE_VIDEO_CAPTURE，表⽰定义了⼀个视频采集 流 类型的buffer。
-fmt---pix为表⽰图形格式的v4l2_pix_format型结构体。需要设定pix⾥的几个变量，
-    pixelformat表⽰采集格式，设置为V4L2_PIX_FMT_YUV420;
-    width、height表⽰图像的宽度、⾼度，以字节为单位;
-    sizeimage表⽰图像所占的存储空间⼤⼩，以字节为单位;
-    bytesperline表⽰每⼀⾏的字节数。
-赋值后，⽤ioctl函数通过这个结构体对fd_ v4l2进⾏设置
- * struct v4l2_format - stream data format
- * @type:	enum v4l2_buf_type; type of the data stream
- * @pix:	definition of an image format
- * @pix_mp:	definition of a multiplanar image format
- * @win:	definition of an overlaid image
- * @vbi:	raw VBI capture or output parameters
- * @sliced:	sliced VBI capture or output parameters
- * @raw_data:	placeholder for future extensions and custom formats
- */
- struct v4l2_format {
-	__u32	 type;
-	union {
-		struct v4l2_pix_format		      pix;     /* V4L2_BUF_TYPE_VIDEO_CAPTURE */
-		struct v4l2_pix_format_mplane     pix_mp;  /* V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE */
-		struct v4l2_window		          win;     /* V4L2_BUF_TYPE_VIDEO_OVERLAY */
-		struct v4l2_vbi_format		      vbi;     /* V4L2_BUF_TYPE_VBI_CAPTURE */
-		struct v4l2_sliced_vbi_format     sliced;  /* V4L2_BUF_TYPE_SLICED_VBI_CAPTURE */
-		__u8	raw_data[200];                   /* user-defined */
-	} fmt;
-};
-
- /*
- *	V I D E O   I M A G E   F O R M A T
- */
-struct v4l2_pix_format {
-	__u32         	width;
-	__u32			height;
-	__u32			pixelformat;
-	__u32			field;		/* enum v4l2_field */
-	__u32           bytesperline;	/* for padding, zero if unused */
-	__u32          	sizeimage; // image memory size
-	__u32			colorspace;	/* enum v4l2_colorspace */
-	__u32			priv;		/* private data, depends on pixelformat */
-};
-
--------------------------------------------------------------------
-/*
- *	M E M O R Y - M A P P I N G   B U F F E R S
- */
-struct v4l2_requestbuffers {
-	__u32			count;     // 缓存数量，也就是说在缓存队列里保持多少张照片
-	__u32			type;		/* 数据流类型 enum v4l2_buf_type
-                                必须永远是V4L2_BUF_TYPE_VIDEO_CAPTURE*/
-	__u32			memory;		/* enum v4l2_memory
-                                V4L2_MEMORY_MMAP 或 V4L2_MEMORY_USERPTR*/
-	__u32			reserved[2];
-};
-    /*分配内存*/
-    struct v4l2_requestbuffers  req;
-    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
-      return -1;
-    }
-
--------------------------------------------------------------
-    /*获取并记录缓存的物理空间*/
-    typedef struct VideoBuffer {
-        void   *start;
-        size_t  length;
-    } VideoBuffer;
-
-    /*
-    //The calloc() function allocates memory for an array of nmemb elements of
-        //size bytes each and returns a pointer to the allocated  memory.
-        //The memory is set to zero.  If nmemb or size is 0, then calloc() returns
-        //either NULL, or a unique pointer value that can later be successfully
-        //passed to free().
-    */
-    VideoBuffer*          buffers = calloc( req.count, sizeof(*buffers) );
-    struct v4l2_buffer    buf;
-    /**
-     * struct v4l2_buffer - video buffer info
-     * @index:	id number of the buffer
-     * @type:	enum v4l2_buf_type; buffer type (type == *_MPLANE for
-     *		multiplanar buffers);
-     * @bytesused:	number of bytes occupied by data in the buffer (payload);
-     *		unused (set to 0) for multiplanar buffers
-     * @flags:	buffer informational flags
-     * @field:	enum v4l2_field; field order of the image in the buffer
-     * @timestamp:	frame timestamp
-     * @timecode:	frame timecode
-     * @sequence:	sequence count of this frame
-     * @memory:	enum v4l2_memory; the method, in which the actual video data is
-     *		passed
-     * @offset:	for non-multiplanar buffers with memory == V4L2_MEMORY_MMAP;
-     *		offset from the start of the device memory for this plane,
-     *		(or a "cookie" that should be passed to mmap() as offset)
-     * @userptr:	for non-multiplanar buffers with memory == V4L2_MEMORY_USERPTR;
-     *		a userspace pointer pointing to this buffer
-     * @fd:		for non-multiplanar buffers with memory == V4L2_MEMORY_DMABUF;
-     *		a userspace file descriptor associated with this buffer
-     * @planes:	for multiplanar buffers; userspace pointer to the array of plane
-     *		info structs for this buffer
-     * @length:	size in bytes of the buffer (NOT its payload) for single-plane
-     *		buffers (when type != *_MPLANE); number of elements in the
-     *		planes array for multi-plane buffers
-     * @input:	input number from which the video data has has been captured
-     *
-     * Contains data exchanged by application and driver using one of the Streaming
-     * I/O methods.
-    */
-    struct v4l2_buffer {
-    	__u32			index;
-    	__u32			type;
-    	__u32			bytesused;
-    	__u32			flags;
-    	__u32			field;
-    	struct timeval		timestamp;
-    	struct v4l2_timecode	timecode;
-    	__u32			sequence;
-
-    	/* memory location */
-    	__u32			memory;
-    	union {
-    		__u32           offset;
-    		unsigned long   userptr;
-    		struct v4l2_plane *planes;
-    		__s32		fd;
-    	} m;
-    	__u32			length;
-    	__u32			reserved2;
-    	__u32			reserved;
-    };
-------------------------------------------------------------------------
-
-    int numBufs = 0;
-    for (; numBufs < req.count; numBufs++) {
-        memset( &buf, 0, sizeof(buf) );
-        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
-        buf.memory = V4L2_MEMORY_MMAP;
-        buf.index = numBufs;
-        // 读取缓存
-        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
-            return -1;
-        }
-
-        buffers[numBufs].length = buf.length;
-        // 转换成相对地址
-        /*
-        void *mmap(void *addr, size_t length, int prot, int flags,
-                  int fd, off_t offset);
-        int munmap(void *addr, size_t length);
-        map or unmap files or devices into memory
-        */
-        buffers[numBufs].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
-            MAP_SHARED,fd, buf.m.offset);
-
-        if (buffers[numBufs].start == MAP_FAILED) {
-            return -1;
-        }
-
-        // 放入缓存队列
-        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
-            return -1;
-        }
-    }
-
-
-    /*处理采集数据*/
-    struct v4l2_buffer buf;
-    memset(&buf,0,sizeof(buf));
-    buf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
-    buf.memory=V4L2_MEMORY_MMAP;
-    buf.index=0;
-
-    //读取缓存
-    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1)
-    {
-        return -1;
-    }
-    //…………视频处理算法
-    printf("");
-    //重新放入缓存队列
-    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
-
-        return -1;
-    }
-
-    munmap(NULL, buf.length);
diff --git a/Makefile b/Makefile
index 9f3db54..5500d1d 100644
--- a/Makefile
+++ b/Makefile
@@ -1,8 +1,9 @@
 
 all:
+	gcc cameraapp.c -o a.out
 	arm-none-linux-gnueabi-gcc cameraapp.c -o camera.app
 	sudo cp -f camera.app  /home/michael/mynfs/rootfs/workspace/camera
 clean:
 	rm -rf *.o *~ core .depend  *.ko *.mod.c .tmp_versions/ Module* modules*
 	sudo rm -f  /home/michael/mynfs/rootfs/workspace/camera/camera.app
-	rm -f camera.app
+	rm -f camera.app a.out
diff --git a/camera.app b/camera.app
deleted file mode 100755
index 0493d0c..0000000
Binary files a/camera.app and /dev/null differ
diff --git a/cameraapp.c b/cameraapp.c
index 790c379..9f4b53d 100644
--- a/cameraapp.c
+++ b/cameraapp.c
@@ -11,30 +11,31 @@
 #include <sys/mman.h>
 
 #define REQ_BUFF_COUNT 5
+#define WIDTH 640
+#define HEIGHT 480
 
-const char* const device_fname = "/dev/video0";
-const char* const output_fname = "catch1";
+static char*  device_fname = "/dev/video0";
+static char*  output_fname = "pic1.jpg";
 static int fd = -1; //摄像头文件描述符
+static struct v4l2_capability cap;  //设备的属性
 typedef struct _buffer  //定义一个结构体来映射每个缓冲帧
 {
     void* start;    //起始地址
     unsigned int length;    //帧长度
 }single_buff;
+
 single_buff* frame_buf = NULL; //(用户空间)记录帧 映射到用户空间(包含4个用户可用的帧)
 
-/**
-    req_buffers: 内核视频缓冲区VIDIOC_REQBUFS,原始数据
+/**req_buffers: 内核视频缓冲区VIDIOC_REQBUFS,原始数据
     (3个以上, 每个存放一帧图像)
     1.  app通过VIDIOC_QUERYBUF可以查询到缓冲区在内核中的长度和偏移量地址;
     2.  用户访问缓冲区需要 地址映射 mmap() 到用户空间才能访问;
 */
 struct v4l2_requestbuffers req_buffers;
 
-void  check_support_fmt(void);
-
 int open_file(const char*const file_name)
 {
-    int retfd =  open(file_name, O_RDWR,  0);
+    int retfd =  open(file_name, O_RDWR);
     if(retfd < 0){
        perror("File open");
        return(-1);
@@ -42,27 +43,8 @@ int open_file(const char*const file_name)
     return retfd;
 }
 
-void check_device_info(void)
-{//获取设备信息,支持的格式
-    printf("Checking Device Info......\n");
-    struct v4l2_capability cap;
-    ioctl(fd,VIDIOC_QUERYCAP, &cap);
-    printf("device name : %s\n", cap.card);
-    printf("device driver : %s\n", cap.driver);
-    printf("device bus_info : %s\n", cap.bus_info);
-    printf("KERNEL_VERSION : %u.%u.%u ;\n", (cap.version>>16)& 0xFF,
-                (cap.version>>8)&0XFF, (cap.version>>0) & 0xFF );
-    printf("device capabilities : %u\n", cap.capabilities);
-    if(V4L2_BUF_TYPE_VIDEO_CAPTURE == cap.capabilities){
-        printf("This device supports iamge.\n");
-    }else{
-        printf("This device does not support iamge.\n");
-    }
-    check_support_fmt();
-}
-
 void  check_support_fmt(void)
-{//VIDIOC_ENUM_FMT // 显⽰所有⽀持的格式
+{//VIDIOC_ENUM_FMT // 查询,显⽰所有⽀持的格式
     printf("Checking Device Supported Format......\n");
     struct v4l2_fmtdesc fmtdesc;
     fmtdesc.index = 0;
@@ -76,7 +58,7 @@ void  check_support_fmt(void)
     struct v4l2_format fmt;
     memset ( &fmt, 0, sizeof(fmt) );
     fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
-    fmt.fmt.pix.pixelformat=V4L2_PIX_FMT_MJPEG;
+    fmt.fmt.pix.pixelformat= V4L2_PIX_FMT_MJPEG;
     if(ioctl(fd,VIDIOC_TRY_FMT,&fmt)==-1 && errno==EINVAL){
         printf("not support format V4L2_PIX_FMT_MJPEG!\n");
     }else{
@@ -85,6 +67,31 @@ void  check_support_fmt(void)
 
 }
 
+
+void check_device_info(void)
+{//获取设备信息,支持的格式
+    printf("Checking Device Info......\n");
+    ioctl(fd,VIDIOC_QUERYCAP, &cap);
+    printf("device name : %s\n", cap.card);
+    printf("device driver : %s\n", cap.driver);
+    printf("device bus_info : %s\n", cap.bus_info);
+    printf("KERNEL_VERSION : %u.%u.%u ;\n", (cap.version>>16)& 0xFF,
+                (cap.version>>8)&0XFF, (cap.version>>0) & 0xFF );
+    printf("device capabilities : %u\n", cap.capabilities);
+    if(V4L2_CAP_VIDEO_CAPTURE  & cap.capabilities){
+        printf("This device DO supports iamge captureing.\n");
+    }else{
+        printf("This device DO NOT support iamge captureing .\n");
+    }
+    if(V4L2_CAP_STREAMING & cap.capabilities){
+        printf("This device DO supports iamge captureing STREAMING.\n");
+    }else{
+        printf("This device DO NOT support iamge captureing STREAMING.\n");
+    }
+    check_support_fmt();
+}
+
+
 void get_current_frame_fmt(void)
 {//查看当前视频帧格式: VIDIOC_G_FMT
     printf("Getting Current Frame Format......\n" );
@@ -94,7 +101,7 @@ void get_current_frame_fmt(void)
 
     ioctl(fd, VIDIOC_G_FMT, &current_fmt);
     printf("Current Frame Format: \n" );
-    printf("\t size of the buffer = %d;\n"
+    printf("\t size of the image = %u;\n"
             "\t width=%u;\n\t height=%u;\n",
                 current_fmt.fmt.pix.sizeimage,
                 current_fmt.fmt.pix.width,
@@ -102,32 +109,33 @@ void get_current_frame_fmt(void)
     if( current_fmt.fmt.pix.field == V4L2_FIELD_INTERLACED){
          printf("Storate format is interlaced frame format\n");
     }
-    printf("Current Format: %ul\n", current_fmt.fmt.pix.pixelformat);
-
-    // struct v4l2_fmtdesc current_fmtdesc;
-    // memset ( &current_fmtdesc, 0, sizeof(current_fmtdesc) );
-    // current_fmtdesc = current_fmt.fmt.pix.pixelformat;
-    // current_fmtdesc.index = 0;
-    // current_fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
-    // while (ioctl(fd, VIDIOC_G_FMT, &current_fmtdesc) != -1) {
-    //     if( current_fmtdesc.pixelformat & current_fmt.fmt.pix.pixelformat){
-    //         printf("Current Format: %s\n", current_fmtdesc.description);
-    //     }
-    //     current_fmtdesc.index++;
-    // }
+    printf("Current Frame Format: %u\n", current_fmt.fmt.pix.pixelformat);
+
+
+    struct v4l2_fmtdesc current_fmtdesc;
+    memset ( &current_fmtdesc, 0, sizeof(current_fmtdesc) );
+    current_fmtdesc.index = 0;
+    current_fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
+    while (ioctl(fd, VIDIOC_ENUM_FMT, &current_fmtdesc) != -1) {
+        if( current_fmtdesc.pixelformat & current_fmt.fmt.pix.pixelformat){
+            printf("\tCurrent Frame Format: %u\n", current_fmt.fmt.pix.pixelformat);
+            printf("\tCurrent Frame Format: %s\n", current_fmtdesc.description);
+            break;
+        }
+        current_fmtdesc.index++;
+    }
 }
 
-int set_catch_frame_fmt(void)
-{//设置捕捉的 帧格式
-    printf("Seting Catch Frame fmt ......\n");
+int set_capture_frame_fmt(void)
+{//设置捕捉的 帧格式(视频制式NTSC/PAL????)
+    printf("Seting capture Frame fmt ......\n");
     struct v4l2_format fmt; //stream data format
     memset ( &fmt, 0, sizeof(fmt) );
     // definition of an image format( struct v4l2_format.fmt.pix)
     fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
-    fmt.fmt.pix.width       = 640;
-    fmt.fmt.pix.height      = 480;
-    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
-    // fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
+    fmt.fmt.pix.width       = WIDTH;
+    fmt.fmt.pix.height      = HEIGHT;
+    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;    //定义pixel format
     fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
 
     // set fmt
@@ -135,22 +143,29 @@ int set_catch_frame_fmt(void)
         perror("set format");
         return -1;    // error return -1
     }
-    printf("Seting Catch Frame fmt Ok!\n");
+    if((fmt.fmt.pix.width == WIDTH) && (fmt.fmt.pix.height == HEIGHT)){
+        printf("Seting capture Frame fmt Ok!\n");
+    }else{
+        printf("Seting capture Frame fmt Failed!\n");
+        return -1;
+    }
     return 0;   // success return 0;
 }
 
 
 int request_buffers(int cnt)
 {//申请视频流缓冲区, 包含cnt个缓存
+    printf("Setting Input/Output Method......\n");
     memset(&req_buffers, 0, sizeof(req_buffers));
-
     req_buffers.count = cnt;
     req_buffers.type =  V4L2_BUF_TYPE_VIDEO_CAPTURE;
     req_buffers.memory = V4L2_MEMORY_MMAP;
+
     if(ioctl(fd, VIDIOC_REQBUFS, &req_buffers) < 0){
         perror("request_buffers: VIDIOC_REQBUFS");
         return -1;
     }
+
     if (req_buffers.count < 2){
         printf("insufficient buffer memory\n");
         printf("Number of buffers allocated = %d\n", req_buffers.count);
@@ -185,10 +200,12 @@ int memory_map(void)
             perror("Memory Mappping--VIDIOC_QUERYBUF");
             return -1;
         }
-        //映射到frame_buf[i].start开始地址,映射到用户空间
+        // 保存长度
         frame_buf[i].length = tmp_buf.length;
+        printf("-----------------tmp_buf.length= %d------------\n", tmp_buf.length);
         printf("frame_buf[%d].length = %u\n", i, frame_buf[i].length);
 
+        //映射到frame_buf[i].start开始地址,映射到用户空间
         frame_buf[i].start =
             mmap(NULL,
                 tmp_buf.length,
@@ -201,33 +218,20 @@ int memory_map(void)
             return -1;
         }
         printf("buffers[%d].start = %#x\n", i, frame_buf[i].start);
-        // 保存长度
-
     }
 }
 
-
-int set_inout_method(void)
-{//申请管理缓冲区
-    //设置输入输出方法(缓冲区管理): 使用内存映射mmap
-    printf("Setting Input/Output Method......\n");
-    //1.申请缓冲区(包含REQ_BUFF_COUNT个帧缓冲)
-    request_buffers(REQ_BUFF_COUNT);
-    //2.获取缓冲帧的地址,长度(通过frame_buf来保存)
-    memory_map();
-}
-
 int add_to_input_queue(void)
 {//把缓冲帧 放入 驱动是视频输入队列
     printf("Adding Frame to Input Queues......\n");
     unsigned int i;
-    struct v4l2_buffer  tmp_buf;
-    memset(&tmp_buf, 0, sizeof(tmp_buf));
-    tmp_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
-    tmp_buf.memory = V4L2_MEMORY_MMAP;
+
     //加入输入队列
     for(i; i < req_buffers.count; i++){
-
+        struct v4l2_buffer  tmp_buf;
+        memset(&tmp_buf, 0, sizeof(tmp_buf));
+        tmp_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
+        tmp_buf.memory = V4L2_MEMORY_MMAP;
 
         tmp_buf.index = i;
         if( ioctl(fd, VIDIOC_QBUF, &tmp_buf) < 0){
@@ -238,45 +242,47 @@ int add_to_input_queue(void)
     return 0;
 }
 
-/**
-    采集数据步骤:
-    1.  把缓冲区req_buffers(5个)放到视频输入队列 VIDIOC_QBUF;
-        输入队列input_queues(等待驱动存放视频的队列)
-    2.  启动视频获取,ioctl:VIDIOC_STREAMON;
-    3.  循环采集连续视频;
-    4.  将输入队列的第一个帧缓冲区 --> 输出队列
-        输出队列output_queues(等待用户读取视频的队列)
-*/
-int start_catch(void)
+
+int start_capture(void)
 {//循环采集数据
-    printf("Starting Catching Frame......\n");
+    /**
+        采集数据步骤:
+        1.  把缓冲区req_buffers(5个)放到视频输入队列 VIDIOC_QBUF;
+            输入队列input_queues(等待驱动存放视频的队列)
+        2.  启动视频获取,ioctl:VIDIOC_STREAMON;
+        3.  循环采集连续视频;
+        4.  将输入队列的第一个帧缓冲区 --> 输出队列
+            输出队列output_queues(等待用户读取视频的队列)
+    */
+    printf("Starting captureing Frame......\n");
 
     enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
-    int ret = 0;
+    int ret = -1;
     ret  = ioctl(fd, VIDIOC_STREAMON, &type);
     if(ret < 0){
-        perror("Starting Catching--VIDIOC_STREAMON");
+        perror("Starting captureing--VIDIOC_STREAMON");
         return ret;
     }
     return ret;
 }
 
-/**
-    数据处理:
-    1.  从输出队列output_queues取出存有视频的帧缓冲req_buffers[i]: VIDIOC_DQBUF
-    2.  处理数据;
-    3.  处理完后, 将该帧缓冲req_buffers[i] 重新放回输入队列input_queues: VIDIOC_QBUF
-    start_catch(); 和 process_image(); 应放在一起实现循环采集处理
-*/
+
 int process_image(void)
 {//处理采集到的帧
+    /**
+        数据处理:
+        1.  从输出队列output_queues取出存有视频的帧缓冲req_buffers[i]: VIDIOC_DQBUF
+        2.  处理数据;
+        3.  处理完后, 将该帧缓冲req_buffers[i] 重新放回输入队列input_queues: VIDIOC_QBUF
+        start_capture(); 和 process_image(); 应放在一起实现循环采集处理
+    */
     printf("Processing Image......\n");
     struct v4l2_buffer  tmp_buf;
     memset(&tmp_buf, 0, sizeof(tmp_buf));
     tmp_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     printf("清空后的buffer类型:%d\n", tmp_buf.type);
     tmp_buf.memory = V4L2_MEMORY_MMAP;
-    printf("xxxxxxxxxxxxx......\n");
+    printf("---------------------------------------\n");
 
     ioctl(fd, VIDIOC_DQBUF, &tmp_buf);
     printf("序号:%u\n", tmp_buf.index);
@@ -287,38 +293,21 @@ int process_image(void)
     printf("缓冲帧地址 :%u\n", tmp_buf.m.offset);
     printf("缓冲帧length:%u\n", tmp_buf.length);
 
-    int wrfd = open(output_fname, O_WRONLY | O_CREAT | O_TRUNC , 0664);
+    int wrfd = open(output_fname, O_RDWR | O_CREAT, 0777);
     if(wrfd < 0){
         perror("output file open");
         return -1;
     }
-    ssize_t wrsize = write(wrfd, &tmp_buf, tmp_buf.bytesused);
-    printf("wrsize = %.3lf kB\n", wrsize);
-
-    // int i = 4;
-    // while(i--){
-    //     ioctl(fd, VIDIOC_DQBUF, &tmp_buf);
-    //     ioctl(fd, VIDIOC_QUERYBUF, &tmp_buf);
-    //     printf("tmp_buf.bytesused = %u Byte\n", tmp_buf.bytesused);
-    //     // //处理数据
-    //     int wrfd = open("pic1.jpg", O_WRONLY | O_CREAT | O_TRUNC , 0664);
-    //     if(wrfd < 0){
-    //         perror("pic1 open");
-    //         return -1;
-    //     }
-    //     ssize_t wrsize = write(wrfd, &tmp_buf, 1024*1024);
-    //     printf("wrsize = %.3lf kB\n", wrsize/1000.0);
-    //
-    //
-    //     //把用过的帧放回输入队列
-    //     if( ioctl(fd, VIDIOC_QBUF, &tmp_buf) < 0){
-    //         perror("Processing Image--VIDIOC_QBUF");
-    //         return -1;
-    //     }
-    // }
 
+    /*
+    问题出现这里, 写的内容要从frame_buf[]这个数组中读取,而不是tmp_buf!!!
+    */
+    ssize_t wrsize = write(wrfd, frame_buf[0].start, frame_buf->length);
+    printf("wrsize = %d kB\n", wrsize/1024);
 
 
+    printf("---------------------------------------\n");
+
     //将取出的帧放回队列
     if( ioctl(fd, VIDIOC_QBUF, &tmp_buf)  < 0){
         perror("Processing Image--VIDIOC_QBUF");
@@ -330,16 +319,17 @@ int process_image(void)
 }
 
 
-/**
-    停止视频采集:
-    1.  停止采集; VIDIOC_STREAMOFF
-    2.  释放 帧缓冲区req_buffers;(unmap)
-    2.  关闭设备;
-*/
-int stop_catch(void)
-{
+
+int stop_capture(void)
+{// 停止采集
+    /**
+        停止视频采集:
+        1.  停止采集; VIDIOC_STREAMOFF
+        2.  释放 帧缓冲区req_buffers;(unmap)
+        2.  关闭设备;
+    */
     enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
-    if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
+    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
         perror("Closing Device--VIDIOC_STREAMOFF");
         return -1;
     }
@@ -354,72 +344,91 @@ void close_device(void)
     for(i; i < req_buffers.count; i++){
         munmap(NULL, frame_buf[i].length);
     }
-    stop_catch();
+    stop_capture();
 	close(fd);
     free(frame_buf);
 }
 
+int device_init()
+{//设备初始化
+    /*1. 设置采集帧格式,并检测设置结果*/
+    set_capture_frame_fmt();
+    get_current_frame_fmt();
 
-
+    /*2. 申请缓存*/
+    request_buffers(REQ_BUFF_COUNT);
+    /*3.获取缓冲帧的地址,长度(通过frame_buf来保存)*/
+    memory_map();
+}
 
 int main(int argc, char* argv[])
 {
     /*1. open video0 device*/
     fd = open_file(device_fname);
-    if(fd < 0){
+    if(fd < 0)
+    {
         printf("File open failed... fd = %d\n", fd );
         exit(-1);
     }else{
         printf("File open successfully... fd = %d\n", fd );
     }
 
-    /*2. 查看设备能力,是否有音视频输入输出?(VIDIOC_QUERYCAP：查询驱动功能)*/
-    check_device_info();
-    get_current_frame_fmt();
-
-    set_catch_frame_fmt();
-    get_current_frame_fmt();
-
-    set_inout_method();
-
-    add_to_input_queue();      //之前没有把缓冲帧加入到 输入队列
-
-    start_catch();
-    process_image();
-
-    // // 7.3 处理缓冲数据(一帧)
-    // struct v4l2_buffer buf;
-    // memset(&buf, 0, sizeof(buf));
-    // buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
-    // buf.memory = V4L2_MEMORY_MMAP;
-    // buf.index = 0;
-    // printf("buf = %#x\n", buf);
-    // printf("从缓冲区取出一帧(数据帧)\n" );
-    // //从缓冲区取出一帧(数据帧)
-    // if(ioctl(fd, VIDIOC_DQBUF, &buf) < 0){
-    //     perror("DQBUF:");
-    //     exit(-1);
+    // /*2. 查看设备能力,是否有音视频输入输出?(VIDIOC_QUERYCAP：查询驱动功能)*/
+    // check_device_info();
+    // get_current_frame_fmt();
+
+    printf("xxxxxxxxxxxxxxxxxxxxxx\n" );
+    // struct v4l2_input cinput;
+    // memset(&cinput, 0, sizeof(cinput));
+
+    // ioctl(fd, VIDIOC_G_INPUT, &cinput.index);//首先获得当前输入的 index,注意只是 index，要获得具体的信息，就的调用列举操作
+    // ioctl (fd, VIDIOC_ENUMINPUT, &cinput);//调用列举操作，获得 input.index 对应的输入的具体信息
+    // printf("Current input is '%s', supports: \n", cinput.name);
+    // std.index = 0;
+    // while (0==ioctl(fd, VIDIOC_ENUMINPUT, &std)) {
+    //     if(std.id & cinput.std)
+    //         printf("%s\n", std.name);
+    //     std.index ++;
     // }
-    // printf("buf = %#x\n", buf);
-    // //处理数据
-    // int wrfd = open("pic1.jpg", O_WRONLY | O_CREAT | O_TRUNC , 0664);
-    // // FILE *fp = fopen("pic2.jp", w+);
-    // if(wrfd < 0){
-    // // if(NULL == fp)
-    //     perror("pic1 open:");
+    // printf("Now input index is : %u\n", cinput.index);
+    // v4l2_std_id std_id;
+    // struct v4l2_standard std;
+    // if(ioctl(fd, VIDIOC_G_STD, &std_id) == -1){
+    //     perror("VIDIOC_G_STD");
     //     exit(-1);
     // }
-    // // ssize_t rdsize = fwrite(buffers[buf.index].start, buf.bytesused, 1, fp);
-    // ssize_t rdsize = write(wrfd, buffers[0].start, buffers[0].length);
-    // printf("read size = %d\n", rdsize);
-    // printf("处理数据\n" );
+    // memset(&std, 0, sizeof(std));
+    // std.index = 0;
+    // while( 0==ioctl(fd, VIDIOC_ENUMSTD, &std) ){
+    //     if(std.id & std_id){
+    //         printf("Current Standard : %s\n", std.name);
+    //         break;
+    //     }
+    //     std.index++;
+    // }
     //
-    // // 将取出的缓冲帧放回缓冲区, 重复利用
-    // if(ioctl(fd, VIDIOC_QBUF, &buf) <0){
-    //     printf("QDBUF Failed. \n");
-    //     exit(-1);
+    // if(errno == EINVAL || std.index == 0 ){
+    //     perror("VIDIOC_ENUMSTD");
+    //     return -1;
     // }
+    //
+    // printf("xxxxxxxxxxxxxxxxxxxxxx\n" );
+
+    // /*3. 设备初始化*/
+    device_init();
+
+    /*4. 把缓存帧添加到缓冲队列*/
+    add_to_input_queue();      //之前没有把缓冲帧加入到 输入队列
+
+    /*4. 开始获取图像*/
+    start_capture();
+    /*5. 处理图片*/
+    process_image();
+    unsigned int i = 0xffffff;
+    while(i--);
+    process_image();
 
+    /*6. 关闭设备*/
     close_device();
 
     return 0;
diff --git a/cp.sh b/cp.sh
deleted file mode 100755
index fdbaea3..0000000
--- a/cp.sh
+++ /dev/null
@@ -1,9 +0,0 @@
-#########################################################################
-# File Name: cp.sh
-# Author: Michael Jay
-# mail: michaelxu2016@yahoo.com
-# Created Time: 2016年08月16日 星期二 16时22分02秒
-# Function: 
-#########################################################################
-#!/bin/bash
-	 cp *.ko  ~/mynfs/rootfs/kotest/ -f
diff --git a/log/v1.png b/log/v1.png
index 7dbf547..5dcfeb1 100644
Binary files a/log/v1.png and b/log/v1.png differ
diff --git a/tmp.c b/tmp.c
deleted file mode 100644
index 839f561..0000000
--- a/tmp.c
+++ /dev/null
@@ -1,373 +0,0 @@
-/*
-*
-VIDIOC_REQBUFS：分配内存
-VIDIOC_QUERYBUF：把VIDIOC_REQBUFS中分配的数据缓存转换成物理地址
-VIDIOC_QUERYCAP：查询驱动功能
-VIDIOC_ENUM_FMT：获取当前驱动支持的视频格式
-VIDIOC_S_FMT：设置当前驱动的频捕获格式
-VIDIOC_G_FMT：读取当前驱动的频捕获格式
-VIDIOC_TRY_FMT：验证当前驱动的显示格式
-VIDIOC_CROPCAP：查询驱动的修剪能力,即缩放
-VIDIOC_S_CROP：设置视频信号的边框
-VIDIOC_G_CROP：读取视频信号的边框
-VIDIOC_QBUF：把数据从缓存中读取出来
-VIDIOC_DQBUF：把数据放回缓存队列
-VIDIOC_STREAMON：开始视频显示 函数
-VIDIOC_STREAMOFF：结束视频显示 函数
-VIDIOC_QUERYSTD：检查当前视频设备支持的标准，例如PAL或NTSC。
-*/
--------------------------------------------------------------------
-struct v4l2_crop {
-	__u32			type;	/* enum v4l2_buf_type */
-	struct v4l2_rect        c;
-};
-struct v4l2_rect {
-	__s32   left;
-	__s32   top;
-	__u32   width;
-	__u32   height;
-};
-
-设置采集窗口就是在摄像头设备的取景范围之内设定⼀个视频采集区域。主要是对结构体v4l2_crop赋
-值，v4l2_crop由⼀个v4l2_buffer_type枚举类型的type和v4l2_rect类型的结构体c构成，
-来描述视频采集窗口的类型和⼤⼩。type设置为视频采集类型V4L2_BUF_TYPE_VIDEO_CAPTURE。
-c是表⽰采集窗口的⼤⼩的结构体(左右上下)，它的成员Left和Top分别表⽰视频采集区域的起始横坐标
-和纵坐标，width和height分别表⽰采集图像的宽度和⾼度。赋值后，⽤ioctl函数通过这个结构体对
-fd进⾏设置。
--------------------------------------------------------
-/** check_device_func();
-  * struct v4l2_capability - Describes V4L2 device caps returned by VIDIOC_QUERYCAP
-  *
-  * @driver:	   name of the driver module (e.g. "bttv")
-  * @card:	   name of the card (e.g. "Hauppauge WinTV")
-  * @bus_info:	   name of the bus (e.g. "PCI:" + pci_name(pci_dev) )
-  * @version:	   KERNEL_VERSION
-  * @capabilities: capabilities of the physical device as a whole
-  * @device_caps:  capabilities accessed via this particular device (node)
-  * @reserved:	   reserved fields for future extensions
-  */
-struct v4l2_capability {
-	__u8	driver[16];
-	__u8	card[32];
-	__u8	bus_info[32];
-	__u32   version;
-	__u32	capabilities;
-	__u32	device_caps;
-	__u32	reserved[3];
-};
-
-------------------------------------------------------
-获取当前支持的格式
-int ioctl(int fd, int request, struct v4l2_fmtdesc *argp);
-struct v4l2_fmtdesc
-{
-    __u32 index; // 要查询的格式序号，应⽤程序设置
-    enum v4l2_buf_type type; // 帧类型，应⽤程序设置
-    __u32 flags; // 是否为压缩格式
-    __u8 description[32]; // 格式名称
-    __u32 pixelformat; // 格式
-    __u32 reserved[4]; // 保留
-};
-_____________________________________________________
-// 查看或设置当前格式
-VIDIOC_G_FMT, VIDIOC_S_FMT
-// 检查是否⽀持某种格式
-VIDIOC_TRY_FMT
-int ioctl(int fd, int request, struct v4l2_format *argp);
-struct v4l2_format
-{
-    enum v4l2_buf_type type;// 帧类型，应⽤程序设置
-    union fmt
-    {
-        struct v4l2_pix_format pix;// 视频设备使⽤
-        struct v4l2_window win;
-        struct v4l2_vbi_format vbi;
-        struct v4l2_sliced_vbi_format sliced;
-        __u8 raw_data[200];
-    };
-};
-struct v4l2_pix_format
-{
-    __u32 width; // 帧宽，单位像素
-    __u32 height; // 帧⾼，单位像素
-    __u32 pixelformat; // 帧格式
-    enum  v4l2_field field;
-    __u32 bytesperline;
-    __u32 sizeimage;
-    enum  v4l2_colorspace colorspace;
-    __u32 priv;
-};
-_______________________________________________________
-图像缩放
-VIDIOC_CROPCAP
-int ioctl(int fd, int request, struct v4l2_cropcap *argp);
-struct v4l2_cropcap
-{
-    enum v4l2_buf_type type;// 应⽤程序设置
-    struct v4l2_rect bounds;// 最⼤边界
-    struct v4l2_rect defrect;// 默认值
-    struct v4l2_fract pixelaspect;
-}
-// 设置缩放
-VIDIOC_G_CROP:读取视频信号的边框
-VIDIOC_S_CROP:设置视频信号的边框
-int ioctl(int fd, int request, struct v4l2_crop *argp);
-int ioctl(int fd, int request, const struct v4l2_crop *argp);
-struct v4l2_crop
-{
-    enum v4l2_buf_type type;// 应⽤程序设置
-    struct v4l2_rect c;
-}
-_______________________________________________________
-
-缓冲区申请管理:
-// 向设备申请缓冲区
-VIDIOC_REQBUFS
-int ioctl(int fd, int request, struct v4l2_requestbuffers *argp);
-struct v4l2_requestbuffers
-{
-    __u32 count; // 缓冲区内缓冲帧的数⽬
-    enum v4l2_buf_type type; // 缓冲帧数据格式
-    enum v4l2_memory memory; // 区别是内存映射还是⽤⼾指针⽅式
-    __u32 reserved[2];
-};
-enum v4l2_memoy {V4L2_MEMORY_MMAP,V4L2_MEMORY_USERPTR};
-//count,type,memory 都要应⽤程序设
-
-//获取缓冲区地址,长度
-获取缓冲帧的地址，长度：
-VIDIOC_QUERYBUF
-int ioctl(int fd, int request, struct v4l2_buffer *argp);
-struct v4l2_buffer
-{
-    __u32 index; //buffer 序号
-    enum v4l2_buf_type type; //buffer 类型
-    __u32 byteused; //buffer 中已使⽤的字节数
-    __u32 flags; // 区分是MMAP 还是USERPTR
-    enum v4l2_field field;
-    struct timeval timestamp;// 获取第⼀个字节时的系统时间
-    struct v4l2_timecode timecode;
-    __u32 sequence; // 队列中的序号
-    enum v4l2_memory memory;//IO ⽅式，被应⽤程序设置
-    union m
-    {
-        __u32 offset;// 缓冲帧地址，只对MMAP 有效
-        unsigned long userptr;
-    };
-    __u32 length;// 缓冲帧长度
-    __u32 input;
-    __u32 reserved;
-};
-//MMAP ，定义⼀个结构体来映射每个缓冲帧。
-Struct buffer
-{
-    void* start;
-    unsigned int length;
-}*buffers;
-
-void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t
-offset);
-//addr 映射起始地址，⼀般为NULL ，让内核⾃动选择
-//length 被映射内存块的长度
-//prot 标志映射后能否被读写，其值为PROT_EXEC,PROT_READ,PROT_WRITE, PROT_NONE
-//flags 确定此内存映射能否被其他进程共享，MAP_SHARED,MAP_PRIVATE
-//offset, 确定被映射的内存地址
-返回成功映射后的地址，不成功返回MAP_FAILED ((void*)-1);
-int munmap(void *addr, size_t length);// 断开映射
-//addr 为映射后的地址，length 为映射后的内存长度
-
-
-_______________________________________________________
-
-
-
-
-
-/**
-    设置视频点阵格式和点阵⼤⼩
-主要是对结构体v4l2_format进⾏赋值，它由type和联合体fmt构成，来描述视频设备当前⾏为和数据的格式。
-type--赋值为视频采集类型V4L2_BUF_TYPE_VIDEO_CAPTURE，表⽰定义了⼀个视频采集 流 类型的buffer。
-fmt---pix为表⽰图形格式的v4l2_pix_format型结构体。需要设定pix⾥的几个变量，
-    pixelformat表⽰采集格式，设置为V4L2_PIX_FMT_YUV420;
-    width、height表⽰图像的宽度、⾼度，以字节为单位;
-    sizeimage表⽰图像所占的存储空间⼤⼩，以字节为单位;
-    bytesperline表⽰每⼀⾏的字节数。
-赋值后，⽤ioctl函数通过这个结构体对fd_ v4l2进⾏设置
- * struct v4l2_format - stream data format
- * @type:	enum v4l2_buf_type; type of the data stream
- * @pix:	definition of an image format
- * @pix_mp:	definition of a multiplanar image format
- * @win:	definition of an overlaid image
- * @vbi:	raw VBI capture or output parameters
- * @sliced:	sliced VBI capture or output parameters
- * @raw_data:	placeholder for future extensions and custom formats
- */
- struct v4l2_format {
-	__u32	 type;
-	union {
-		struct v4l2_pix_format		      pix;     /* V4L2_BUF_TYPE_VIDEO_CAPTURE */
-		struct v4l2_pix_format_mplane     pix_mp;  /* V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE */
-		struct v4l2_window		          win;     /* V4L2_BUF_TYPE_VIDEO_OVERLAY */
-		struct v4l2_vbi_format		      vbi;     /* V4L2_BUF_TYPE_VBI_CAPTURE */
-		struct v4l2_sliced_vbi_format     sliced;  /* V4L2_BUF_TYPE_SLICED_VBI_CAPTURE */
-		__u8	raw_data[200];                   /* user-defined */
-	} fmt;
-};
-
- /*
- *	V I D E O   I M A G E   F O R M A T
- */
-struct v4l2_pix_format {
-	__u32         	width;
-	__u32			height;
-	__u32			pixelformat;
-	__u32			field;		/* enum v4l2_field */
-	__u32           bytesperline;	/* for padding, zero if unused */
-	__u32          	sizeimage; // image memory size
-	__u32			colorspace;	/* enum v4l2_colorspace */
-	__u32			priv;		/* private data, depends on pixelformat */
-};
-
--------------------------------------------------------------------
-/*
- *	M E M O R Y - M A P P I N G   B U F F E R S
- */
-struct v4l2_requestbuffers {
-	__u32			count;     // 缓存数量，也就是说在缓存队列里保持多少张照片
-	__u32			type;		/* 数据流类型 enum v4l2_buf_type
-                                必须永远是V4L2_BUF_TYPE_VIDEO_CAPTURE*/
-	__u32			memory;		/* enum v4l2_memory
-                                V4L2_MEMORY_MMAP 或 V4L2_MEMORY_USERPTR*/
-	__u32			reserved[2];
-};
-    /*分配内存*/
-    struct v4l2_requestbuffers  req;
-    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
-      return -1;
-    }
-
--------------------------------------------------------------
-    /*获取并记录缓存的物理空间*/
-    typedef struct VideoBuffer {
-        void   *start;
-        size_t  length;
-    } VideoBuffer;
-
-    /*
-    //The calloc() function allocates memory for an array of nmemb elements of
-        //size bytes each and returns a pointer to the allocated  memory.
-        //The memory is set to zero.  If nmemb or size is 0, then calloc() returns
-        //either NULL, or a unique pointer value that can later be successfully
-        //passed to free().
-    */
-    VideoBuffer*          buffers = calloc( req.count, sizeof(*buffers) );
-    struct v4l2_buffer    buf;
-    /**
-     * struct v4l2_buffer - video buffer info
-     * @index:	id number of the buffer
-     * @type:	enum v4l2_buf_type; buffer type (type == *_MPLANE for
-     *		multiplanar buffers);
-     * @bytesused:	number of bytes occupied by data in the buffer (payload);
-     *		unused (set to 0) for multiplanar buffers
-     * @flags:	buffer informational flags
-     * @field:	enum v4l2_field; field order of the image in the buffer
-     * @timestamp:	frame timestamp
-     * @timecode:	frame timecode
-     * @sequence:	sequence count of this frame
-     * @memory:	enum v4l2_memory; the method, in which the actual video data is
-     *		passed
-     * @offset:	for non-multiplanar buffers with memory == V4L2_MEMORY_MMAP;
-     *		offset from the start of the device memory for this plane,
-     *		(or a "cookie" that should be passed to mmap() as offset)
-     * @userptr:	for non-multiplanar buffers with memory == V4L2_MEMORY_USERPTR;
-     *		a userspace pointer pointing to this buffer
-     * @fd:		for non-multiplanar buffers with memory == V4L2_MEMORY_DMABUF;
-     *		a userspace file descriptor associated with this buffer
-     * @planes:	for multiplanar buffers; userspace pointer to the array of plane
-     *		info structs for this buffer
-     * @length:	size in bytes of the buffer (NOT its payload) for single-plane
-     *		buffers (when type != *_MPLANE); number of elements in the
-     *		planes array for multi-plane buffers
-     * @input:	input number from which the video data has has been captured
-     *
-     * Contains data exchanged by application and driver using one of the Streaming
-     * I/O methods.
-    */
-    struct v4l2_buffer {
-    	__u32			index;
-    	__u32			type;
-    	__u32			bytesused;
-    	__u32			flags;
-    	__u32			field;
-    	struct timeval		timestamp;
-    	struct v4l2_timecode	timecode;
-    	__u32			sequence;
-
-    	/* memory location */
-    	__u32			memory;
-    	union {
-    		__u32           offset;
-    		unsigned long   userptr;
-    		struct v4l2_plane *planes;
-    		__s32		fd;
-    	} m;
-    	__u32			length;
-    	__u32			reserved2;
-    	__u32			reserved;
-    };
-------------------------------------------------------------------------
-
-    int numBufs = 0;
-    for (; numBufs < req.count; numBufs++) {
-        memset( &buf, 0, sizeof(buf) );
-        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
-        buf.memory = V4L2_MEMORY_MMAP;
-        buf.index = numBufs;
-        // 读取缓存
-        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
-            return -1;
-        }
-
-        buffers[numBufs].length = buf.length;
-        // 转换成相对地址
-        /*
-        void *mmap(void *addr, size_t length, int prot, int flags,
-                  int fd, off_t offset);
-        int munmap(void *addr, size_t length);
-        map or unmap files or devices into memory
-        */
-        buffers[numBufs].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
-            MAP_SHARED,fd, buf.m.offset);
-
-        if (buffers[numBufs].start == MAP_FAILED) {
-            return -1;
-        }
-
-        // 放入缓存队列
-        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
-            return -1;
-        }
-    }
-
-
-    /*处理采集数据*/
-    struct v4l2_buffer buf;
-    memset(&buf,0,sizeof(buf));
-    buf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
-    buf.memory=V4L2_MEMORY_MMAP;
-    buf.index=0;
-
-    //读取缓存
-    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1)
-    {
-        return -1;
-    }
-    //…………视频处理算法
-    printf("");
-    //重新放入缓存队列
-    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
-
-        return -1;
-    }
-
-    munmap(NULL, buf.length);
