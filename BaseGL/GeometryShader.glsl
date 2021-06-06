#version 450 core // Minimal GL version support expected from the GPU
#extension GL_EXT_geometry_shader4 : enable

layout( points) in; // Inputs points
layout( line_strip, max_vertices= 2) out; // Output line strips
void main(){
    for(int i = 0; i < 2; i++) {
        gl_Position= gl_in[ i ].gl_Position;
        EmitVertex(); // New vertex generated
    }
    EndPrimitive(); // New primitive generated using the last 2 emitted vertices
}