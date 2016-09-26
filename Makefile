SOURCES := 	xmain.cpp \
		audio.cpp \
		bsp.cpp \
		decal.cpp \
		edge.cpp \
		engine.cpp \
		entity.cpp \
		frame.cpp \
		graphics.cpp \
		gltLoadTGA.cpp \
		light.cpp \
		matrix.cpp \
		md5.cpp \
		md5model.cpp \
		model.cpp \
		menu.cpp \
		net.cpp \
		parse.cpp \
		plane.cpp \
		player.cpp \
		quaternion.cpp \
		rigidbody.cpp \
		shader.cpp \
		sin_table.cpp \
		speaker.cpp \
		trigger.cpp \
		vector.cpp \
		vehicle.cpp \


OBJS  := $(SOURCES:%.cpp=%.o)
OBJDIR := $(SOURCES:%.cpp=obj/%.o)

%.o: src/%.cpp
	$(CPP) $(CFLAGS) -c $(INCLUDES) -o obj/$@ $<


INCLUDES = -I./include/ -I/usr/local/opt/openal-soft/include -I/usr/X11R6/include -I/opt/X11/include 
#CPP := g++
CPP := clang++

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

CFLAGS := -DGL_GLEXT_PROTOTYPES -Wno-write-strings -Wall -O1 -g -fsanitize=address -fno-omit-frame-pointer
LFLAGS_OSX := -lX11 -lGL -lGLU -lc -framework OpenAL
LFLAGS := -lX11 -lGL -lGLU -lopenal
LIBS := -L/usr/X11R6/lib/ 

all: altEngine

altEngine: $(OBJS)
	$(CPP) $(CFLAGS) -o altEngine $(OBJDIR) $(LIBS) $(LFLAGS)

clean:
	rm -f ./obj/*.o
	rm -f ./altEngine

