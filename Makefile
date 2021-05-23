NUMJOBS=$(shell nproc)

SOURCES_CPP := 	xmain.cpp \
		audio.cpp \
		bsp_q3.cpp \
		bsp_q1.cpp \
		bsp_hl.cpp \
		cloth.cpp \
		common.cpp \
		engine.cpp \
		ent_constructable.cpp \
		ent_light.cpp \
		ent_model.cpp \
		ent_player.cpp \
		ent_portal_camera.cpp \
		ent_projectile.cpp \
		ent_rigidbody.cpp \
		ent_speaker.cpp \
		ent_trigger.cpp \
		ent_vehicle.cpp \
		entity.cpp \
		frame.cpp \
		graph.cpp \
		graphics_opengl.cpp \
		graphics_software.cpp \
		graphics_vulkan.cpp \
		hashtable.cpp \
		heap.cpp \
		intersect.cpp \
		isocube.cpp \
		isosphere.cpp \
		junzip.cpp \
		matrix.cpp \
		md5.cpp \
		md5model.cpp \
		menu.cpp \
		netcode.cpp \
		parse.cpp \
		pak.cpp \
		quake3.cpp \
		quaternion.cpp \
		raster.cpp \
		serial.cpp \
		shader_d3d9.cpp \
		shader_d3d11.cpp \
		shader_opengl.cpp \
		shader_software.cpp \
		shader_vulkan.cpp \
		shadow_volume.cpp \
		sin_table.cpp \
		sock.cpp \
		sph.cpp \
		spline.cpp \
		terrain.cpp \
		vector.cpp \
		vm.cpp \
		vmmain.cpp \
		voice.cpp \

SOURCES_CC :=	huffman.c \
		md5sum.c \

OBJDIR_CPP  := $(SOURCES_CPP:%.cpp=obj/%.o)
OBJDIR_C  := $(SOURCES_CC:%.c=obj/%.o)


obj/%.o: src/%.cpp
	$(CPP) $(CFLAGS) -c $(INCLUDES) -o $@ $<

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $(INCLUDES) -o $@ $<


INCLUDES = -I./include/ -I/usr/local/opt/openal-soft/include -I/usr/X11R6/include -I/opt/X11/include 
CPP := g++ -fuse-linker-plugin -std=c++11 -Wfloat-equal
CC := gcc -fuse-linker-plugin -Wfloat-equal

#CPP := clang++ -std=c++11
#CC := clang

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

#gdb will stop on SIG34, type "handle SIG34 nostop noprint pass" to fix
# put that setting in ~/.gdbinit to not have to type it each time
# Press Control-C to re-enter gdb
# type "break Engine::" then press Tab to see list of class break points
# Press Control-X Control-A to enter TUI graphical mode
# obvious n for step over, and s for step into
# display to display a variable

altEngine: CFLAGS := -MMD -MP -DGL_GLEXT_PROTOTYPES -DSOFTWARE  -Wno-write-strings -Wall -g -march=native -fno-exceptions -fno-rtti -ffast-math -ffunction-sections #-fsanitize=address -fno-omit-frame-pointer
altEngine_dedicated: CFLAGS := -flto -DGL_GLEXT_PROTOTYPES -DDEDICATED -DOPENGL -Wno-write-strings -Wall -march=native -fno-exceptions -fno-rtti
altEngine_vulkan: CFLAGS := -flto -DGL_GLEXT_PROTOTYPES -DVULKAN -Wno-write-strings -Wall -march=native -fno-exceptions -fno-rtti
LFLAGS_OSX := -lX11 -lGL -lc -framework OpenAL
LFLAGS := -lX11 -lGL -lopenal -lrt -lpthread -lopus -ldl
LFLAGS_VULKAN := -lX11 -lGL -lvulkan -lrt -lpthread
LIBS := -L/usr/X11R6/lib/ -L/usr/local/lib

.PHONY: clean


all: altEngine

altEngine: $(OBJDIR_CPP) $(OBJDIR_C)
	$(CPP) $(CFLAGS) -o altEngine $(OBJDIR_CPP) $(OBJDIR_C) $(LIBS) $(LFLAGS)

altEngine_vulkan: $(OBJDIR_CPP) $(OBJDIR_C)
	$(CPP) $(CFLAGS) -o altEngine_vulkan $(OBJDIR_CPP) $(OBJDIR_C) $(LIBS) $(LFLAGS_VULKAN)

altEngine_dedicated: $(OBJDIR_CPP) $(OBJDIR_C)
	$(CPP) $(CFLAGS) -o altEngine_dedicated $(OBJDIR_CPP) $(OBJDIR_C) $(LIBS) $(LFLAGS)

clean:
	rm -f ./obj/*.o
	rm -f ./obj/*.d
	rm -f ./altEngine

-include $(SOURCES_CPP:%.cpp=obj/%.d)
-include $(SOURCES_CC:%.c=obj/%.d)
