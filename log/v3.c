
错误排查:
1.	Segmentation fault(开发板)
在PC上运行没有段错误,但是在开发板上会出现段错误;
原因: 可能是内核版本不同, munmap

如下代码有问题:

	//关闭 映射时, 这里的NULL应该是之前申请的地址的首地址
    for(i; i < req_buffers.count; i++){
        if(munmap(NULL, frame_buf[i].length)<0){
            perror("munmap");
            exit -1;
        }
    }
    
改成:
	首地址就是frame_buf[i].start;
    for(i; i < req_buffers.count; i++){
        if(munmap( frame_buf[i].start, frame_buf[i].length)<0){
            perror("munmap");
            exit -1;
        }
    }
    
2.	在开发板生成的yuyv格式文件, 不能通过YUYV查看器直接查看(linux环境下)
原因：	权限问题，通过nfs过载的文件系统,(~/mynfs/rootfs/)创建的文件在PC端直接通过软件访问,需要先复制到PC端,然后在打开查看,否则看不到采集到的图像;
