Real-time fiber-level Cloth Rendering project
===========================

This is one of the projects I chose to work on throughout my studies in Computer Graphics. I implement part of what is described in the paper **Real-time fiber-level Cloth Rendering by Kui Wu, Cem Yuksel (I3D 2017).**

To do so, I also relied on a basic **OpenGL codebase provided by Tamy BOUBEKEUR**, which is referenced in the main.cpp file. The codebase permitted to showcase a mesh in real-time, and I built upon that to be able to showcase fiber-level cloth.

Most of the additions can be found in the shaders, but some nonetheless very important can be found in the main.cpp file.

If you have any questions or inquieries, please leave me a message. **Also, please do mention me and the authors of the paper if you happen to use the code !**

CHeers !

### Running the project

## Method 1 

I wrote the commands into a commands.txt file. 
If using a linux system, you can do the following: 

```
cd <path-to-BaseGL-directory>
bash commands 
```

## Method 2
To run the code, run these commands:

```
cd <path-to-BaseGL-directory>
mkdir build
cd build
cmake ..
cmake --build .
cd ..
./BaseGL
```

The executable is automatically copied to the root BaseGL directory, so that shaders can be loaded (thus explaining the two last commands) 

