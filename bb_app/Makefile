CC_C = arm-linux-gnueabihf-gcc
#CC_C = gcc
CFLAGS = -Wall -g -std=c99 -D_POSIX_C_SOURCE=200809L #-Werror
OFLAGS = -pthread
SOURCES = main.c stall.c washroom.c network.c joystick.c file.c led.c
TARGET = reserver
TARGET_DIR = $(HOME)/cmpt433/public/myApps


all: server
	$(CC_C) $(CFLAGS) $(SOURCES) -o $(TARGET_DIR)/$(TARGET) -lpthread

server:
	mkdir -p $(TARGET_DIR)/reserver-server-copy/
	cp -R ../server_site/* $(TARGET_DIR)/reserver-server-copy/

clean:
	rm $(TARGET_DIR)/reserver
	rm main.o washroom.o stall.o network.o