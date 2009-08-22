gcc -o altEngine -g -I../include bsp.cpp engine.cpp entity.cpp frame.cpp gltLoadTGA.cpp graphics.cpp matrix.cpp quaternion.cpp sound.cpp vector.cpp voxel.cpp xmain.cpp -lGL -lGLU -lopenal -lX11
mv altEngine ../
