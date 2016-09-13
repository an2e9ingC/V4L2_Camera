all:
	gcc cameraapp.c -o a.out -ljpeg
	arm-none-linux-gnueabi-gcc cameraapp.c -o camera.app -ljpeg
	sudo cp -f camera.app  /home/michael/mynfs/rootfs/workspace/camera
clean:
	rm -rf *.o *~ core .depend  *.ko *.mod.c .tmp_versions/ Module* modules*
	sudo rm -f  /home/michael/mynfs/rootfs/workspace/camera/camera.app
	rm -f camera.app a.out
