# Adamas Framework

This is a study project of closs-platform game framework.
Currently, this framework supports following platforms.

* Android - OpenGL ES 2.0
* Windows - OpenGL ES 3.1
* Windows - Vulkan
* Windows - DirectX 11
* Windows - DirectX 12

# Submodule required

This project refers Lua 5.3 as a git submodule.
Please type following from git bash to obtain submodule.

> git submodule init

> git submodule update

# How to build for Windows platform

Just open VisualStudio\Adamas.sln and build it.
Build for Vulkan, the SDK is required. Visit https://vulkan.lunarg.com/ for download.
Build for OpenGL ES 3.1, OpenGL extension header files required. Please refer VisualStudio\glheaders\place_extension_headers_here.txt for more information.

# How to build for Android platform

Android Studio required. Open AndroidStudio directory with Android Studio.
