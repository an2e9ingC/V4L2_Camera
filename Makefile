# .PHONY:all
# all: a.out

a.out: camera.o main.o camera.h
	gcc main.o camera.o -o a.out -ljpeg
main.o:
	gcc main.c -c -o main.o
camera.o:
	gcc camera.c -c -o camera.o

# all:
	# gcc main.c -o a.out -ljpeg
	# arm-none-linux-gnueabi-gcc main.c -o camera.app -ljpeg
	#sudo cp -f camera.app  /home/michael/mynfs/rootfs/workspace/camera
.PHONY:clean
clean:
	# sudo rm -f  /home/michael/mynfs/rootfs/workspace/camera/camera.app
	rm -f camera.app a.out
	rm -f *.o
