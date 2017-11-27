NUMJOBS := 8

SOURCES_CPP := 	main.cpp \
		net.cpp \

#SOURCES_CC :=	huffman.c \
		md5sum.c \


OBJS_CPP  := $(SOURCES_CPP:%.cpp=%.o)
OBJDIR_CPP := $(SOURCES_CPP:%.cpp=obj/%.o)
OBJS_C  := $(SOURCES_CC:%.c=%.o)
OBJDIR_C := $(SOURCES_CC:%.c=obj/%.o)

%.o: src/%.cpp
	$(CPP) $(CFLAGS) -c $(INCLUDES) -o obj/$@ $<

%.o: src/%.c
	$(CC) $(CFLAGS) -c $(INCLUDES) -o obj/$@ $<


INCLUDES = -I./include/ 
#CPP := g++ -fuse-linker-plugin -std=c++11 -Wfloat-equal
#CC := gcc -fuse-linker-plugin -Wfloat-equal
CPP := clang++ -std=c++11
CC := clang

#coverity stuff, OSX has g++ point to clang, so must use linux for coverity run
#cov-configure --comptype gcc --compiler [path to compiler]
#cov-build --dir cov-int make

#AddressSanitizer
#-O1 -g -fsanitize=address -fno-omit-frame-pointer

# clang memory sanitizer -- ie the slow one
#-fsanitize=memory -fno-omit-frame-pointer
#-fsanitize-memory-track-origins
#-fsanitize-memory-use-after-dtor
#-fsanitize=safe-stack

master: CFLAGS := -Wno-write-strings -Wall -O3 -march=native -fno-exceptions -fno-rtti #-fsanitize=address -fno-omit-frame-pointer
LFLAGS := -lrt -lpthread 
LIBS := -L/usr/local/lib

all: master

master: $(OBJS_CPP)
	$(CPP) $(CFLAGS) -o master $(OBJDIR_CPP) $(LIBS) $(LFLAGS)

clean:
	rm -f ./obj/*.o
	rm -f ./master

