Real-time fiber-level Cloth Rendering project
===========================

### Running

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

