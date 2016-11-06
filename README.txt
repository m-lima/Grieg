INF251 - Zohar Malamant & Marcelo Lima

NOTES for Assignment 3:
The following features work:

- Lights are now calculated correctly

- Textures load as expected

- The model can be rotated with R

- The ambient, diffuse and specular coefficients are loaded from the .mtl file
  for each material.

- Bump mapping works. Replace Grieghallen with Suzanne using M.

- Animated textures work. First, replace Grieghallen with Suzanne using M, and
  then replace the bump mapping with animated water using W.

- Alternate shading functions exist and can be changed with S. We have basic
  shading (just Phong shading), toon shading and tilt-shift shading.

OLD NOTES for Assignment 2:
The model loading and textures work. The process is not very optimised, but it does its job.

The camera also works as expected:
- Left mouse button: Translate view
- Right mouse button: Rotate view
- Middle mouse button: Reset camera
- Spacebar: Toggle perspective
- Alt+Enter: Toggle fullscreen

The lights are a last minute addition, and do not work very well.
There should be ambient, diffuse and (broken?) specular light. As well as a directional light (works),
point light (works) and spotlight (kind of works). The suzanne model acts as the source of the light.
There is a hint of attenuation (ie. darkness because the light is too far away), but I couldn't get
the coefficients right.

The program supports multiple objects and shaders, although we didn't take advantage of this fact.

COMPILING:
I provided a .sln in the /msvc directory. Hopefully it'll work without needing to do anything special.

Dependencies:
- CMake 3.6
- Python 2.7
Make sure both are in %PATH%.

Additionally, the following dependencies are already provided:
- SDL2 (in /extern)
- SDL2_image (in /extern)
- glm (in /glm)
- glad (in /glad)
- fmtlib (int /fmt)

Creating a sln from cmd/powershell:
Navigate to the source root (ie. the directory in which CMakeLists.txt resides)
> mkdir .\build
> cd .\build
> cmake ..
CMake should automatically detect Visual Studio 14 and create a .sln in the .\build directory.
You can optionally compile the project from the command line:
> cmake --build . --target Release
