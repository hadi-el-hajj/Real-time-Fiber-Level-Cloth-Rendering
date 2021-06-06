#version 450 core // Minimal GL version support expected from the GPU

layout(location = 0) in vec3 aPos;
layout(location =1) in vec3 aNormals; 
out vec3 vPos;
out vec3 vNormals; 

void main() {
    vPos = aPos;
    vNormals = aNormals; 
}