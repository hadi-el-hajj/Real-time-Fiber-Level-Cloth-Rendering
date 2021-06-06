#version 450 core // Minimal GL version support expected from the GPU

in vec4 fcolor ; 
out vec4 color; // Shader output: the color response attached to this fragment


void main() {
	color = fcolor;
}