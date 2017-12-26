## Synopsis

altEngine2 is my 3d engine, it loads quake3 BSP maps and renders them in OpenGL 4.4

## Download

http://lineofsight.awright2009.com/downloads.html

## Compiling

Linux:
	Linux will need glx, opengl development headers, openal sdk (freealut should work)
	Once you have prereq's just type make

Windows:
	You will need glew:
		http://glew.sourceforge.net/
	OpenAL SDK
		https://www.openal.org/downloads/

	Open altEngine2.sln (currently using visual studio 2015)

	Should compile after pointing VC++ Directories to both the include and lib directories of glew and openal sdk
	(Sometimes I check in stuff that doesnt compile though, and I dont always update the project files diligently, but it should compile most of the time)


## Misc info

	D3D9 will compile and render as well, but the shaders havent been updated in some time
	D3D11 needs work, think it might render the menu currently
	vulkan needs work, it will only render the menu background
	oculus vr will render, but doesnt currently handle user input (from oculus touch or head movements)