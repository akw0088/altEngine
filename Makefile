SOURCES := 	src/xmain.cpp \
		src/audio.cpp \
		src/bsp.cpp \
		src/decal.cpp \
		src/edge.cpp \
		src/engine.cpp \
		src/entity.cpp \
		src/frame.cpp \
		src/graphics.cpp \
		src/gltLoadTGA.cpp \
		src/light.cpp \
		src/matrix.cpp \
		src/md5.cpp \
		src/md5model.cpp \
		src/model.cpp \
		src/menu.cpp \
		src/net.cpp \
		src/parse.cpp \
		src/plane.cpp \
		src/player.cpp \
		src/quaternion.cpp \
		src/rigidbody.cpp \
		src/shader.cpp \
		src/sin_table.cpp \
		src/speaker.cpp \
		src/trigger.cpp \
		src/vector.cpp \
		src/vehicle.cpp \


OBJS := $(SOURCES:.cpp=.o)


INCLUDES = -I./include/ -I/usr/local/opt/openal-soft/include -I/usr/X11R6/include -I/opt/X11/include 
#CPP := g++
CPP := clang++
CFLAGS := -DGL_GLEXT_PROTOTYPES -Wno-write-strings -Wall
LFLAGS := -lGL -lGLU -lopenal -lX11
LIBS := 

all: altEngine

altEngine: $(OBJS)
	$(CPP) $(CFLAGS) -o altEngine $(OBJS) $(LFLAGS) $(LIBS)

clean:
	rm -f ./src/*.o


%.o:%.cpp
	$(CPP) $(CFLAGS) -c $(INCLUDES) -o $@ $<
