如何使用库，详情阅读　"libjpeg.txt"
You should read at least the "Overview" and "Basic library usage" sections before trying
to program with the library.  The sections on advanced features can be read
if and when you need them.

Overview:
	Functions provided by the library
	Outline of typical usage
Basic library usage:
	Data formats
	Compression details
	Decompression details
	Mechanics of usage: include files, linking, etc


OVERVIEW
========

Functions provided by the library
---------------------------------
The IJG JPEG library provides C code to read and write JPEG-compressed image
files.  The surrounding application program receives or supplies image data a
scanline(扫描线) at a time, using a straightforward uncompressed image format.  All
details of color conversion and other preprocessing/postprocessing can be
handled by the library.
提供读写压缩的JPEG图片文件的c源码.
直接通过未解压的图片格式,为app同时提供 收/发 一个扫描线的图片数据???
所有的color转化细节和预处理/后处理 都可以通过lib来处理;

一个程序中一般使用8bit或者12bit的一中处理方式;


Outline of typical usage
------------------------
"压缩的操作流程:"
The rough outline of a JPEG compression operation is:

	"1. 重定位并初始化压缩目标"
	"Allocate and initialize a JPEG compression object"
	压缩目标: "struct jpeg_compress_struct". (在一个程序中:使用局部变量; 否则:static/malloc);
	错误处理: "struct jpeg_error_mgr" 默认错误处理,如果严重错误,会直接调用exit();
				"必须初始化"
				"首先把他保存到一个指针,指向压缩目标的 struct jpeg_compress_struct.err,"
				然后调用"jpeg_create_compress()"初始化目标结构体的其他部分;
		套路:
			struct jpeg_compress_struct cinfo;
			struct jpeg_error_mgr jerr;
			...
			cinfo.err = jpeg_std_error(&jerr);
			//申请一段较小的内存,因此有可能失败(通过error handler退出,所以要先初始化),需要判断
			jpeg_create_compress(&cinfo);




	"2. 为压缩数据确定目标(一般用文件)"
	"Specify the destination for the compressed data (eg, a file)"
		在使用标准目标模块时, 必须提前打开目标文件

			FILE * outfile;
			...
			if ((outfile = fopen(filename, "wb")) == NULL) {	//以二进制格式打开
				fprintf(stderr, "can't open %s\n", filename);
				exit(1);
			}
			jpeg_stdio_dest(&cinfo, outfile);	//invokes(调用,激活,援引) the standard destination module.



			WARNING:
			"it is critical that the binary compressed data be delivered to the"
			"output file unchanged." On non-Unix systems the stdio library may perform
			newline translation or otherwise corrupt binary data.  To suppress this
			behavior, you may need to use a "b" option to fopen (as shown above), or use
			setmode() or another routine to put the stdio stream in binary mode.  See
			cjpeg.c and djpeg.c for code that has been found to work on many systems.

			"也可以在设置完参数(step3)后在选择数据输出目标, 如果这样的话,就不要在jpeg_start_compress()
			"和jpeg_finish_compress()之间 改变destination module.




	"3. 设置压缩参数, 包括: 图片大小, 颜色空间 "
	"Set parameters for compression, including image size & colorspace"
		必须提供源图片的信息,通过JPEG对象(struct jpeg_compress_struct cinfo)

				image_width			Width of image, in pixels
				image_height		Height of image, in pixels
				input_components	Number of color channels (samples per pixel)"颜色通道数量"
				in_color_space		Color space of source image"源图片的颜色空间"

			"JPEG支持的尺寸:每个方向支持 1-64k 个像素
			"颜色通道数量: RGB-3, Gray-1
			"输入颜色空间: 必须是J_COLOR_SPACE枚举类型,主要包括:JCS_RGB,JCS_GRAYSCALE

		由于JPEG的压缩参数很多, 大部分可以使用默认值,通过　"jpeg_set_defaults()"
		如果有特别需要设置的，就在这个设置默认值操作后进行即可；
		The "Compression parameter selection"　section tells about all the parameters.

		"注意:
			1. "在设置默认值之前，一定要正确设置in_color_space,因为默认的设置是根据这个参数来确定的；
			2. "其他三个参数是在jpeg_start_compress()才会使用到；
			3. "当然 jpeg_set_defaults()可以被多次调用

		套路:
			Typical code for a 24-bit RGB source image is

			cinfo.image_width = Width; 	/* image width and height, in pixels */
			cinfo.image_height = Height;
			cinfo.input_components = 3;	/* # of color components per pixel */
			cinfo.in_color_space = JCS_RGB; /* colorspace of input image */

			jpeg_set_defaults(&cinfo);
			/* Make optional parameter settings here */




	"4. 开始处理jpeg_start_compress压缩"
	"jpeg_start_compress(...);"
		在 "建立数据目标" 及"设置好必要的源图片参数"后,就可以调用"jpeg_start_compress()开始循环压缩了"
		This will initialize internal state, allocate working storage, and emit the first few bytes of the JPEG datastream header.

		套路:
			jpeg_start_compress(&cinfo, TRUE);


			"The "TRUE" parameter ensures that a complete JPEG interchange datastream will be written."


	"5. 当扫描线被保持被写入状态时, 写入scanlines "
	"while (scan lines remain to be written)"
	"jpeg_write_scanlines(...);"

		"通过一次或者多次调用jpeg_write_scanlines()来写入需要的图片数据
		一次调用可以传送 1个或多个scanlines,上限是图片的总height, 当然一次写1行或者几行比较好
		图片数据使用"top-to-bottom"的顺序;(cjpeg例子中也有bottom-to-top的方式)"

		库包含一个 "当前已经写入的scanlines的数量计数器count, 这个数量会保存到JPEG对象的next_scanline成员中"
		因此, 可以使用这个变量作为"循环计数器loop counter",使用方法:
			"while (cinfo.next_scanline < cinfo.image_height)".
		这步操作很大程度上依赖你储存源数据的方式,"example.c"展示了 full-size 二维源数组包含3-byte RGB像素:

		套路:
			JSAMPROW row_pointer[1];	/* pointer to a single row */
			int row_stride;			/* physical row width in buffer */

			row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */

			while (cinfo.next_scanline < cinfo.image_height) {
				row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
				jpeg_write_scanlines(&cinfo, row_pointer, 1);
			}


			jpeg_write_scanlines(); 返回值实际写入的scanlines的数量, 一般就是写入的



	"6. 结束压缩"
	"jpeg_finish_compress(...);	"
		所有的数据全部写完后, 调用"jpeg_finish_compress()"结束压缩循环, 同时释放占用的内存;
		"这一步很有必要, 它能确认最后一个bufferload是否已经都写入目标"

		套路:
			"jpeg_finish_compress(&cinfo);"
		如果使用stdio目标,别忘了关闭输出流.
		如果要提前结束压缩, 使用jpeg_abort();
		结束一个循环后, 可以继续处理另一个图像,也可像step7中释放压缩对象.
		如果继续,就返回 step 2,3,4 合适的步骤继续;
		如果不更改输出管理,那么输出的数据流会被写入到相同的目标中;
		如果不更改JPEG的参数,那么参数还是和之前的一样
		"但是记住,每个循环之后,都可以更改这些参数的"
		"但是,如果更改了,尤其是 in_color_space, 那么必须要重新调用一次jpeg_set-defaults()"
		然后就重复step3即可;







	"7. 释放压缩对象"
	"Release the JPEG compression object"
		处理完一个目标的压缩后,要销毁:
			"jpeg_destroy_compress();  只是销毁,不管之前目标的状态"
			"jpeg_destroy();	同时销毁压缩/解压缩的目标,对于压缩/解压缩共享一段代码的方便"

		"注意:  如果jpeg_compress_struct structure from malloc(), 这个不会释放,需要自己释放.	"

		套路:
			"jpeg_destroy_compress(&cinfo);"


	"8. 终止压缩: Aborting"
		两种终止方法:
			1.	如果"不需要继续使用JPEG对象"的,直接调用"jpeg_destroy_compress()/jpeg_destroy();"
			"这两个很安全,即使jpeg_create_compress();失败了."
			2.	如果还"要重复使用JPEG对象"的,调用"jpeg_abort_compress()/jpeg_abort();"
				"jpeg_abort_compress();//只终止压缩"
				"jpeg_abort(): //压缩/解压都终止"

		"注意: 使用动态分配的内存还是要自己回收"




"压缩对象保存库的参数和工作状态"

	"创建销毁对象是和 start/finish 压缩分开的;"
	同一个对象可被一系列的压缩操作重复使用的;(这就队处理一系列的图片方便了,使用同一个参数即可)

	被压缩的图像数据被提供给 "jpeg_write_scanlines()" 内核缓存;
	如果应用程序要实现 从文件到文件的压缩, 那么读取图像数据就该有应用处理;
	库 会通过调用"data destination manager 来发送已经压缩的数据"
	(这个manager通常要把数据写入到文件中, 当然应用也可以
	提供"自己特定的destination manager"去完成别的事)


"解压缩的操作流程:(与压缩类似)"

	Allocate and initialize a JPEG decompression object

	Specify the source of the compressed data (eg, a file)

	Call jpeg_read_header() to obtain image info
	"调用 jpeg_read_header() 获取图片信息"

	Set parameters for decompression

	jpeg_start_decompress(...);

	while (scan lines remain to be read)
		jpeg_read_scanlines(...);

	jpeg_finish_decompress(...);

	Release the JPEG decompression object



"图像的数据格式:"


	The standard input image format is a rectangular array of pixels, with each
	pixel having the same number of "component成分" or "sample例子(颜色通道)" values (color
	channels).  You must specify how many components there are and the colorspace
	interpretation(解释说明) of the components.  Most applications will use RGB data
	(three components per pixel) or grayscale data (one component per pixel).

	PLEASE NOTE THAT RGB DATA IS THREE SAMPLES PER PIXEL, GRAYSCALE ONLY ONE.
	"RGB 每个像素3个组成成分red,blue,green"
	"gray 灰度: 每个像素1个成分"

	A remarkable number of people manage to miss this, only to find that their
	programs don't work with grayscale JPEG files.

	There is no provision for colormapped input.  JPEG files are always full-color
	or full grayscale (or sometimes another colorspace such as CMYK).  You can
		"不提供彩色映射的输入, JPEG是全彩/全灰度的"
	"feed in输入" a colormapped image by expanding it to full-color format.  However
	JPEG often doesn't work very well with source data that has been colormapped,
	because of "dithering抖动" noise.
	This is discussed in more detail in the JPEG FAQ
	and the other references mentioned in the README file.

	Pixels are stored by scanlines, with each scanline running from left to
	right.  The component values for each pixel are "adjacent临近的" in the row; for
	example, R,G,B,R,G,B,R,G,B,... for 24-bit RGB color.  Each scanline is an
	array of data type JSAMPLE --- which is typically "unsigned char", unless
	you have changed jmorecfg.h.  (You can also change the RGB pixel layout, say
	to B,G,R order, by modifying jmorecfg.h.  But see the restrictions listed in
	that file before doing so.)

	A 2-D array of pixels is formed by making a list of pointers to the starts of
	scanlines; so the scanlines need not be physically adjacent in memory.  Even
	if you process just one scanline at a time, you must make a one-element
	pointer array to "conform 遵守" to this structure.  Pointers to JSAMPLE rows are of
	type JSAMPROW, and the pointer to the pointer array is of type JSAMPARRAY.

	The library accepts or supplies one or more complete scanlines per call.
	"库可以在每次调用 接受/提供 1个/多个完整的扫描线"
	It is not possible to process part of a row at a time.Scanlines are always
	processed top-to-bottom.  You can process an entire image in one call if you
	have it all in memory, but usually it's simplest to process one scanline at
	a time.
	"但是 一次处理一个JSAMPROW的一部分是不可以的, 扫描线一般是从头到尾处理的, 可以在一次"
	"调用中处理一张完整图片(只要内存允许), 但是最单的是一次处理一条扫描线"

	For best results, source data values should have the precision specified by
	BITS_IN_JSAMPLE (normally 8 bits).  For instance, if you choose to compress
	data that's only 6 bits/channel, you should left-justify each value in a
	byte before passing it to the compressor.  If you need to compress data
	that has more than 8 bits/channel, compile with BITS_IN_JSAMPLE = 12.
	(See "Library compile-time options", later.)
	"最好,数据有一个8bit的前缀 BIT_IN_JSAMPLE"
	"举个例子,如果你选择压缩  6bits/channel 的图片, 就需要在数据传入压缩器前向左对齐"
	"如果需要压缩大于8bit/channel, 那么宏要改成BITS_IN_JSAMPLE=12"

	The data format returned by the decompressor is the same in all details,
	except that colormapped output is supported.  (Again, a JPEG file is never
	colormapped.  But you can ask the decompressor to perform on-the-fly color
	quantization to deliver colormapped output.)  If you request colormapped
	output then the returned data array contains a single JSAMPLE per pixel;
	its value is an index into a color map.  The color map is represented as
	a 2-D JSAMPARRAY in which each row holds the values of one color component,
	that is, colormap[i][j] is the value of the i`th color component for pixel
	value (map index) j.  Note that since the colormap indexes are stored in
	JSAMPLEs, the maximum number of colors is limited by the size of JSAMPLE
	(ie, at most 256 colors for an 8-bit JPEG library).
	"解压缩类似, 数据 放在二维数组中,每一行row都保存一个颜色的组成成分"
	"colormap[i][j] = 第i个颜色的第[j](map 下标)像素"
	"注意: 因为 colormap的下标index保存在JSAMPLE中, 最大的颜色数量<=256(对于8bitJPEG)"



"头文件包含:
Mechanics of usage: include files, linking, etc
-----------------------------------------------
#include <jpeglib.h>
#include <stdio.h>
#include <<sys/types.h>	//size_t
#include <jerror.h>

The basic control flow for buffered-image decoding is

	jpeg_create_decompress()
	set data source
	jpeg_read_header()
	set overall decompression parameters
	cinfo.buffered_image = TRUE;	/* select buffered-image mode */
	jpeg_start_decompress()
	for (each output pass) {
	    adjust output decompression parameters if required
	    jpeg_start_output()		/* start a new output pass */
	    for (all scanlines in image) {
	        jpeg_read_scanlines()
	        display scanlines
	    }
	    jpeg_finish_output()	/* terminate output pass */
	}
	jpeg_finish_decompress()
	jpeg_destroy_decompress()
