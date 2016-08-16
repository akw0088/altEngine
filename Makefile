
SOURCES :=	src/audio.cpp \
		src/bsp.cpp \
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
		src/sin_table.cpp \
		src/speaker.cpp \
		src/trigger.cpp \
		src/vector.cpp \
		src/vehicle.cpp \
		src/xmain.cpp


OBJS := $(SOURCES:.cpp=.o)


INCLUDES = -I./include/
CPP := g++
CFLAGS := 
LFLAGS := -lGL -lGLU
LIBS := 

all: altEngine

altEngine: $(OBJS)
	$(CPP) $(CFLAGS) -o altEngine $(OBJS) $(LFLAGS) $(LIBS)


%.o:%.cpp
	$(CPP) $(CFLAGS) -c $(INCLUDES) -o $@ $<
