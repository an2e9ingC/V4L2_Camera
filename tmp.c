/*
*
VIDIOC_REQBUFS：分配内存
VIDIOC_QUERYBUF：把VIDIOC_REQBUFS中分配的数据缓存转换成物理地址
VIDIOC_QUERYCAP：查询驱动功能
VIDIOC_ENUM_FMT：获取当前驱动支持的视频格式
VIDIOC_S_FMT：设置当前驱动的频捕获格式
VIDIOC_G_FMT：读取当前驱动的频捕获格式
VIDIOC_TRY_FMT：验证当前驱动的显示格式
VIDIOC_CROPCAP：查询驱动的修剪能力,即缩放
VIDIOC_S_CROP：设置视频信号的边框
VIDIOC_G_CROP：读取视频信号的边框
VIDIOC_QBUF：把数据从缓存中读取出来
VIDIOC_DQBUF：把数据放回缓存队列
VIDIOC_STREAMON：开始视频显示 函数
VIDIOC_STREAMOFF：结束视频显示 函数
VIDIOC_QUERYSTD：检查当前视频设备支持的标准，例如PAL或NTSC。
*/
-------------------------------------------------------------------
struct v4l2_crop {
	__u32			type;	/* enum v4l2_buf_type */
	struct v4l2_rect        c;
};
struct v4l2_rect {
	__s32   left;
	__s32   top;
	__u32   width;
	__u32   height;
};

设置采集窗口就是在摄像头设备的取景范围之内设定⼀个视频采集区域。主要是对结构体v4l2_crop赋
值，v4l2_crop由⼀个v4l2_buffer_type枚举类型的type和v4l2_rect类型的结构体c构成，
来描述视频采集窗口的类型和⼤⼩。type设置为视频采集类型V4L2_BUF_TYPE_VIDEO_CAPTURE。
c是表⽰采集窗口的⼤⼩的结构体(左右上下)，它的成员Left和Top分别表⽰视频采集区域的起始横坐标
和纵坐标，width和height分别表⽰采集图像的宽度和⾼度。赋值后，⽤ioctl函数通过这个结构体对
fd进⾏设置。
-------------------------------------------------------
/** check_device_func();
  * struct v4l2_capability - Describes V4L2 device caps returned by VIDIOC_QUERYCAP
  *
  * @driver:	   name of the driver module (e.g. "bttv")
  * @card:	   name of the card (e.g. "Hauppauge WinTV")
  * @bus_info:	   name of the bus (e.g. "PCI:" + pci_name(pci_dev) )
  * @version:	   KERNEL_VERSION
  * @capabilities: capabilities of the physical device as a whole
  * @device_caps:  capabilities accessed via this particular device (node)
  * @reserved:	   reserved fields for future extensions
  */
struct v4l2_capability {
	__u8	driver[16];
	__u8	card[32];
	__u8	bus_info[32];
	__u32   version;
	__u32	capabilities;
	__u32	device_caps;
	__u32	reserved[3];
};

------------------------------------------------------
获取当前支持的格式
int ioctl(int fd, int request, struct v4l2_fmtdesc *argp);
struct v4l2_fmtdesc
{
    __u32 index; // 要查询的格式序号，应⽤程序设置
    enum v4l2_buf_type type; // 帧类型，应⽤程序设置
    __u32 flags; // 是否为压缩格式
    __u8 description[32]; // 格式名称
    __u32 pixelformat; // 格式
    __u32 reserved[4]; // 保留
};
_____________________________________________________
// 查看或设置当前格式
VIDIOC_G_FMT, VIDIOC_S_FMT
// 检查是否⽀持某种格式
VIDIOC_TRY_FMT
int ioctl(int fd, int request, struct v4l2_format *argp);
struct v4l2_format
{
    enum v4l2_buf_type type;// 帧类型，应⽤程序设置
    union fmt
    {
        struct v4l2_pix_format pix;// 视频设备使⽤
        struct v4l2_window win;
        struct v4l2_vbi_format vbi;
        struct v4l2_sliced_vbi_format sliced;
        __u8 raw_data[200];
    };
};
struct v4l2_pix_format
{
    __u32 width; // 帧宽，单位像素
    __u32 height; // 帧⾼，单位像素
    __u32 pixelformat; // 帧格式
    enum  v4l2_field field;
    __u32 bytesperline;
    __u32 sizeimage;
    enum  v4l2_colorspace colorspace;
    __u32 priv;
};
_______________________________________________________
图像缩放
VIDIOC_CROPCAP
int ioctl(int fd, int request, struct v4l2_cropcap *argp);
struct v4l2_cropcap
{
    enum v4l2_buf_type type;// 应⽤程序设置
    struct v4l2_rect bounds;// 最⼤边界
    struct v4l2_rect defrect;// 默认值
    struct v4l2_fract pixelaspect;
}
// 设置缩放
VIDIOC_G_CROP:读取视频信号的边框
VIDIOC_S_CROP:设置视频信号的边框
int ioctl(int fd, int request, struct v4l2_crop *argp);
int ioctl(int fd, int request, const struct v4l2_crop *argp);
struct v4l2_crop
{
    enum v4l2_buf_type type;// 应⽤程序设置
    struct v4l2_rect c;
}
_______________________________________________________

缓冲区申请管理:
// 向设备申请缓冲区
VIDIOC_REQBUFS
int ioctl(int fd, int request, struct v4l2_requestbuffers *argp);
struct v4l2_requestbuffers
{
    __u32 count; // 缓冲区内缓冲帧的数⽬
    enum v4l2_buf_type type; // 缓冲帧数据格式
    enum v4l2_memory memory; // 区别是内存映射还是⽤⼾指针⽅式
    __u32 reserved[2];
};
enum v4l2_memoy {V4L2_MEMORY_MMAP,V4L2_MEMORY_USERPTR};
//count,type,memory 都要应⽤程序设

//获取缓冲区地址,长度
获取缓冲帧的地址，长度：
VIDIOC_QUERYBUF
int ioctl(int fd, int request, struct v4l2_buffer *argp);
struct v4l2_buffer
{
    __u32 index; //buffer 序号
    enum v4l2_buf_type type; //buffer 类型
    __u32 byteused; //buffer 中已使⽤的字节数
    __u32 flags; // 区分是MMAP 还是USERPTR
    enum v4l2_field field;
    struct timeval timestamp;// 获取第⼀个字节时的系统时间
    struct v4l2_timecode timecode;
    __u32 sequence; // 队列中的序号
    enum v4l2_memory memory;//IO ⽅式，被应⽤程序设置
    union m
    {
        __u32 offset;// 缓冲帧地址，只对MMAP 有效
        unsigned long userptr;
    };
    __u32 length;// 缓冲帧长度
    __u32 input;
    __u32 reserved;
};
//MMAP ，定义⼀个结构体来映射每个缓冲帧。
Struct buffer
{
    void* start;
    unsigned int length;
}*buffers;

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t
offset);
//addr 映射起始地址，⼀般为NULL ，让内核⾃动选择
//length 被映射内存块的长度
//prot 标志映射后能否被读写，其值为PROT_EXEC,PROT_READ,PROT_WRITE, PROT_NONE
//flags 确定此内存映射能否被其他进程共享，MAP_SHARED,MAP_PRIVATE
//offset, 确定被映射的内存地址
返回成功映射后的地址，不成功返回MAP_FAILED ((void*)-1);
int munmap(void *addr, size_t length);// 断开映射
//addr 为映射后的地址，length 为映射后的内存长度


_______________________________________________________





/**
    设置视频点阵格式和点阵⼤⼩
主要是对结构体v4l2_format进⾏赋值，它由type和联合体fmt构成，来描述视频设备当前⾏为和数据的格式。
type--赋值为视频采集类型V4L2_BUF_TYPE_VIDEO_CAPTURE，表⽰定义了⼀个视频采集 流 类型的buffer。
fmt---pix为表⽰图形格式的v4l2_pix_format型结构体。需要设定pix⾥的几个变量，
    pixelformat表⽰采集格式，设置为V4L2_PIX_FMT_YUV420;
    width、height表⽰图像的宽度、⾼度，以字节为单位;
    sizeimage表⽰图像所占的存储空间⼤⼩，以字节为单位;
    bytesperline表⽰每⼀⾏的字节数。
赋值后，⽤ioctl函数通过这个结构体对fd_ v4l2进⾏设置
 * struct v4l2_format - stream data format
 * @type:	enum v4l2_buf_type; type of the data stream
 * @pix:	definition of an image format
 * @pix_mp:	definition of a multiplanar image format
 * @win:	definition of an overlaid image
 * @vbi:	raw VBI capture or output parameters
 * @sliced:	sliced VBI capture or output parameters
 * @raw_data:	placeholder for future extensions and custom formats
 */
 struct v4l2_format {
	__u32	 type;
	union {
		struct v4l2_pix_format		      pix;     /* V4L2_BUF_TYPE_VIDEO_CAPTURE */
		struct v4l2_pix_format_mplane     pix_mp;  /* V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE */
		struct v4l2_window		          win;     /* V4L2_BUF_TYPE_VIDEO_OVERLAY */
		struct v4l2_vbi_format		      vbi;     /* V4L2_BUF_TYPE_VBI_CAPTURE */
		struct v4l2_sliced_vbi_format     sliced;  /* V4L2_BUF_TYPE_SLICED_VBI_CAPTURE */
		__u8	raw_data[200];                   /* user-defined */
	} fmt;
};

 /*
 *	V I D E O   I M A G E   F O R M A T
 */
struct v4l2_pix_format {
	__u32         	width;
	__u32			height;
	__u32			pixelformat;
	__u32			field;		/* enum v4l2_field */
	__u32           bytesperline;	/* for padding, zero if unused */
	__u32          	sizeimage; // image memory size
	__u32			colorspace;	/* enum v4l2_colorspace */
	__u32			priv;		/* private data, depends on pixelformat */
};

-------------------------------------------------------------------
/*
 *	M E M O R Y - M A P P I N G   B U F F E R S
 */
struct v4l2_requestbuffers {
	__u32			count;     // 缓存数量，也就是说在缓存队列里保持多少张照片
	__u32			type;		/* 数据流类型 enum v4l2_buf_type
                                必须永远是V4L2_BUF_TYPE_VIDEO_CAPTURE*/
	__u32			memory;		/* enum v4l2_memory
                                V4L2_MEMORY_MMAP 或 V4L2_MEMORY_USERPTR*/
	__u32			reserved[2];
};
    /*分配内存*/
    struct v4l2_requestbuffers  req;
    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
      return -1;
    }

-------------------------------------------------------------
    /*获取并记录缓存的物理空间*/
    typedef struct VideoBuffer {
        void   *start;
        size_t  length;
    } VideoBuffer;

    /*
    //The calloc() function allocates memory for an array of nmemb elements of
        //size bytes each and returns a pointer to the allocated  memory.
        //The memory is set to zero.  If nmemb or size is 0, then calloc() returns
        //either NULL, or a unique pointer value that can later be successfully
        //passed to free().
    */
    VideoBuffer*          buffers = calloc( req.count, sizeof(*buffers) );
    struct v4l2_buffer    buf;
    /**
     * struct v4l2_buffer - video buffer info
     * @index:	id number of the buffer
     * @type:	enum v4l2_buf_type; buffer type (type == *_MPLANE for
     *		multiplanar buffers);
     * @bytesused:	number of bytes occupied by data in the buffer (payload);
     *		unused (set to 0) for multiplanar buffers
     * @flags:	buffer informational flags
     * @field:	enum v4l2_field; field order of the image in the buffer
     * @timestamp:	frame timestamp
     * @timecode:	frame timecode
     * @sequence:	sequence count of this frame
     * @memory:	enum v4l2_memory; the method, in which the actual video data is
     *		passed
     * @offset:	for non-multiplanar buffers with memory == V4L2_MEMORY_MMAP;
     *		offset from the start of the device memory for this plane,
     *		(or a "cookie" that should be passed to mmap() as offset)
     * @userptr:	for non-multiplanar buffers with memory == V4L2_MEMORY_USERPTR;
     *		a userspace pointer pointing to this buffer
     * @fd:		for non-multiplanar buffers with memory == V4L2_MEMORY_DMABUF;
     *		a userspace file descriptor associated with this buffer
     * @planes:	for multiplanar buffers; userspace pointer to the array of plane
     *		info structs for this buffer
     * @length:	size in bytes of the buffer (NOT its payload) for single-plane
     *		buffers (when type != *_MPLANE); number of elements in the
     *		planes array for multi-plane buffers
     * @input:	input number from which the video data has has been captured
     *
     * Contains data exchanged by application and driver using one of the Streaming
     * I/O methods.
    */
    struct v4l2_buffer {
    	__u32			index;
    	__u32			type;
    	__u32			bytesused;
    	__u32			flags;
    	__u32			field;
    	struct timeval		timestamp;
    	struct v4l2_timecode	timecode;
    	__u32			sequence;

    	/* memory location */
    	__u32			memory;
    	union {
    		__u32           offset;
    		unsigned long   userptr;
    		struct v4l2_plane *planes;
    		__s32		fd;
    	} m;
    	__u32			length;
    	__u32			reserved2;
    	__u32			reserved;
    };
------------------------------------------------------------------------

    int numBufs = 0;
    for (; numBufs < req.count; numBufs++) {
        memset( &buf, 0, sizeof(buf) );
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = numBufs;
        // 读取缓存
        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
            return -1;
        }

        buffers[numBufs].length = buf.length;
        // 转换成相对地址
        /*
        void *mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset);
        int munmap(void *addr, size_t length);
        map or unmap files or devices into memory
        */
        buffers[numBufs].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
            MAP_SHARED,fd, buf.m.offset);

        if (buffers[numBufs].start == MAP_FAILED) {
            return -1;
        }

        // 放入缓存队列
        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
            return -1;
        }
    }


    /*处理采集数据*/
    struct v4l2_buffer buf;
    memset(&buf,0,sizeof(buf));
    buf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory=V4L2_MEMORY_MMAP;
    buf.index=0;

    //读取缓存
    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1)
    {
        return -1;
    }
    //…………视频处理算法
    printf("");
    //重新放入缓存队列
    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {

        return -1;
    }

    munmap(NULL, buf.length);
