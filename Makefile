CC = gcc -g


encoder: main.o camera.o yuv_saver.o encoder_x264.o microphone.o push_stream.o
		gcc -g $^  -o encoder -lSDL2 -lx264 -lSDL2_image -lavutil -lasound -lavformat -lavcodec -lswscale -lswresample -lpthread -lrtmp

# player: decVideo.o main.o 
# 		gcc -g decVideo.o  main.o  -o player -lSDL2 -lSDL2_image -lavformat -lavutil -lavcodec -lswscale -lswresample

main.o:	main.c
	gcc -g -c $^ -lSDL2 -lx264

camera.o: camera.c
	gcc -g -c $^

yuv_saver.o : yuv_saver.c
	gcc -g -c $^

encoder_x264.o :encoder_x264.c
	gcc -g -c $^ -lx264

microphone.o :microphone.c
	gcc -g -c $^ -lasound

push_stream.o :push_stream.c
	gcc -g -c $^ 

.PHONY: clean
clean: 
	-rm *.o encoder 

