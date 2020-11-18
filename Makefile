CC = gcc -g


encoder: main.o camera.o
		gcc -g $^  -o encoder -lSDL2 -lSDL2_image -lavutil -lavformat -lavcodec -lswscale -lswresample -lpthread

# player: decVideo.o main.o 
# 		gcc -g decVideo.o  main.o  -o player -lSDL2 -lSDL2_image -lavformat -lavutil -lavcodec -lswscale -lswresample

main.o:	main.c
	gcc -g -c $^ -lSDL2

camera.o: camera.c
	gcc -g -c $^


.PHONY: clean
clean: 
	-rm *.o encoder 

