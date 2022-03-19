# Goal Based Pathfinding in OpenGL
This program was written in C language, using OpenGL, GLFW and GLEW


### Requirements
- Cmake - minimum required **version 3.11.0**
- Windows:
	> MinGW, **gcc** compiler. C standart **99**.
- Linux:
	>  **gcc** compiler. C standart **99**.
	> Makefile


### Compilation
- You should be able to compile it easily using **.bat** file on Windows or **.sh** if it is Linux
- There is also `makefile` with targets:
	> `build` - create a build folder with the makefile
	> `main` - compile the application in Release mode
	> `mainDebug` - compile the application in Debug mode
	> `clean` - remove build and bin folder with all files to start over


### Modules
For this project I am using GLFW for creating a window and GLEW to manage OpenGL.
These are added as 3rd party submodules.


### Buttons
There are a few responsive program buttons:
| KEY | Action |
|-------------------|---------------------------------|
|*[* and *]* |`creating and deleting particles`|
|*K* and *L* |`particles size` |
|*Left mouse button*|`adding or removing obstacles` |