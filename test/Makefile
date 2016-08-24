#cross=arm-hisiv300-linux-
CC=$(cross)gcc
CPP=$(cross)g++
ZBASE_PATH=../../zbaselib/lib/
ZEVENT_PATH=../lib/
CFLAGS = -g -Wall
LDFLAGS=-lzevent -L$(ZEVENT_PATH) -lzbase -L$(ZBASE_PATH)  -Wl,-rpath,$(ZBASE_PATH):$(ZEVENT_PATH) -lpthread
INCLUDES = -I$(ZBASE_PATH)/include/ -I./ -I$(ZEVENT_PATH)/include/
CFLAGS += $(INCLUDES)

C_SRC = $(wildcard *.c)
C_OBJ = $(patsubst %c, %o, $(C_SRC))
CPP_SRC = $(wildcard *.cpp)
CPP_OBJ = $(patsubst %cpp, %o, $(CPP_SRC))

.PHONY:all clean

all:$(CPP_OBJ) $(C_OBJ)

.c.o:
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) 
.cpp.o:
	$(CPP) $(CFLAGS) -o $@ $< $(LDFLAGS)


clean:
	rm *~ *.o -f
