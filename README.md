# Adamas Framework

This is a study project of closs-platform game framework.
Currently, this framework supports following platforms.

* Android - OpenGL ES 3.1
* Windows - OpenGL ES 3.1
* Windows - DirectX 11
* Windows - DirectX 12

# Submodule required

This project refers Lua 5.3 as a git submodule.
Please type following from git bash to obtain submodule.

> git submodule init

> git submodule update

# How to build for Windows platform

Visual Studio 2015 required. For DX11, just open VisualStudio\Adamas.sln and build it.
Build for OpenGL ES 3.1, OpenGL extension header files required. Please refer VisualStudio\glheaders\place_extension_headers_here.txt for more information.

# How to build for Android platform

Android Studio required. Open AndroidStudio directory with Android Studio.
