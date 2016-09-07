michael@cdut:~/workspace/project/camera$ ./a.out 
File open successfully... fd = 3
Checking Device Info......
device name : USB 2.0 VGA Camera 
device driver : uvcvideo
device bus_info : usb-0000:00:12.0-1
KERNEL_VERSION : 4.4.13 ;
device capabilities : 2216689665
This device DO supports iamge captureing.
This device DO supports iamge captureing STREAMING.
Checking Device Supported Format......
Supported Formats:
	1.Motion-JPEG
Do support format V4L2_PIX_FMT_MJPEG!
Seting capture Frame fmt ......
Seting capture Frame fmt Ok!
Getting Current Frame Format......
Current Frame Format: 
	 size of the image = 921600;
	 width=640;
	 height=480;
Current Frame Format: 1196444237
	Current Frame Format: 1196444237
	Current Frame Format: Motion-JPEG
Setting Input/Output Method......
Memory Mapping...
calloc ok. buffers addr = 0x19dd420
-----------------tmp_buf.length= 921600------------
frame_buf[0].length = 921600
buffers[0].start = 0x73442000
-----------------tmp_buf.length= 921600------------
frame_buf[1].length = 921600
buffers[1].start = 0x7335b000
-----------------tmp_buf.length= 921600------------
frame_buf[2].length = 921600
buffers[2].start = 0x72e6c000
-----------------tmp_buf.length= 921600------------
frame_buf[3].length = 921600
buffers[3].start = 0x72d85000
-----------------tmp_buf.length= 921600------------
frame_buf[4].length = 921600
buffers[4].start = 0x72c9e000
Starting captureing Frame......
Processing Image......
---------------------------------------
序号:0
buffer类型:1
bytesused:18468
flags:73733
sequence队列序号:0
缓冲帧地址 :0
缓冲帧length:921600
wrsize = 900 kB
---------------------------------------
Processed Image ok......
Closing Device fd = 3...





[root@farsight camera]# ./camera.app 
File open successfully... fd = 3
Checking Device Info......
device name : USB 2.0 VGA Camera 
device driver : uvcvideo
device bus_info : usb-12580000.ehci-3.3
KERNEL_VERSION : 3.14.25 ;
device capabilities : 2214592513
This device DO supports iamge captureing.
This device DO supports iamge captureing STREAMING.
Checking Device Supported Format......
Supported Formats:
        1.YUV 4:2:2 (YUYV)
Do support format V4L2_PIX_FMT_MJPEG!
Seting capture Frame fmt ......
Seting capture Frame fmt Ok!
Getting Current Frame Format......
Current Frame Format: 
         size of the image = 614400;
         width=640;
         height=480;
Current Frame Format: 1448695129
        Current Frame Format: 1448695129
        Current Frame Format: YUV 4:2:2 (YUYV)
Setting Input/Output Method......                                                    
Memory Mapping...                                                                    
calloc ok. buffers addr = 0x12008                                                    
-----------------tmp_buf.length= 614400------------                                  
frame_buf[0].length = 614400
buffers[0].start = 0xb6ddf000
-----------------tmp_buf.length= 614400------------
frame_buf[1].length = 614400
buffers[1].start = 0xb6d49000
-----------------tmp_buf.length= 614400------------
frame_buf[2].length = 614400
buffers[2].start = 0xb6cb3000
-----------------tmp_buf.length= 614400------------
frame_buf[3].length = 614400
buffers[3].start = 0xb6c1d000
-----------------tmp_buf.length= 614400------------
frame_buf[4].length = 614400
buffers[4].start = 0xb6b87000
Starting captureing Frame......
Processing Image......
---------------------------------------
��序��号:0
buffer��类��型:1
bytesused:614400
flags:8197
sequence��队��列��序��号:0
��缓��冲��帧��地��址 :0
��缓��冲��帧length:614400
wrsize = 600 kB
---------------------------------------
Processed Image ok......
Closing Device fd = 3...
Segmentation fault

