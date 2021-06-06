#version 450 core // Minimal GL version support expected from the GPU

layout(vertices = 4) out; // 4 points per patch
in vec3 vPos[];
in vec3 vNormals[]; 
out vec3 tcPos[];
out vec3 tcNormals[]; 

void main() {
    tcPos[gl_InvocationID] = vPos[gl_InvocationID];
    tcNormals[gl_InvocationID] = vNormals[gl_InvocationID]; 
    if(gl_InvocationID == 0) { // levels only need to be set once per patch
        gl_TessLevelOuter[0] = 64; // we're only tessellating one line
        gl_TessLevelOuter[1] = 64; // tessellate the line into 100 segments
    }
}