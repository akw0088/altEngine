## Synopsis

altEngine2 is my 3d engine, it loads quake3 BSP maps and renders them in OpenGL 4.4. It also supports D3D9, D3D11, and software rendering. (but they will look noticably worse) Oculus should render, but will not have head tracking / touch tracking, Vulkan will only render the menu background and needs work (rendering only a single vertex buffer, once multiples work everything should render, havent messed with it in a while and back when I did the driver support was very spotty (works on one machine, not the other, changes to fix the other made the first stop working etc)

## Download

http://lineofsight.awright2009.com/downloads.html

## Videos

http://youtube.com/akw0088 (Has some videos related, not all though)

## Compiling

Linux:
	Linux will need glx, opengl development headers, openal sdk (freealut should work)
	Once you have prereq's just type make

	Note: defaulting to Software rendering use -DOPENGL instead of -DSOFTWARE in the makefile for opengl
	(left software as a default as it has less dependencies)

Windows:
	Open altEngine2.sln (currently using visual studio 2015)

	Should compile after installing prereq's
	(Sometimes I check in stuff that doesnt compile though, and I dont always update the project files diligently, but it should compile most of the time)

	To run it you'll need the "media" folder, which you can get from the download links
 
 Mac OSX
    Install homebrew
    Install glew
        brew install glew
    Xcode should compile
    Set working directory to a release directory in Product -> Scheme -> Edit Scheme
        Options Tab
            Working Directory

    Then running it should be fine
    

## Misc info

	D3D9 will compile and render, but the shaders havent been updated in some time, menu might not render, but it's there (again shaders)
	D3D11 will compile and render, but shaders havent been updated in some time, menu might not render, but it's there (again shaders)
	vulkan needs work, it will only render the menu background
	oculus vr will render, but doesnt currently handle user input (from oculus touch or head movements)


## Dependency Download links

GLEW opengl wrangler (microsoft poorly supports opengl, so this is required)  
	http://glew.sourceforge.net/  

DirectX SDK June 2010 (DirectX9) 
	https://www.microsoft.com/en-us/download/details.aspx?id=6812  

Windows SDK (DirectX11)  
	https://developer.microsoft.com/en-us/windows/downloads/sdk-archive/  

OpenAL Sound API  
	https://www.openal.org/downloads/  

Opus Voice Codec  
	https://opus-codec.org/  

Vulkan SDK  
	https://vulkan.lunarg.com/  

This isn't really a dependency, but almost added it. (Text to Speech)  
	http://www.speech.cs.cmu.edu/flite/index.html  

