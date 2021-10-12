#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 aColor; // the color variable has attribute position 1

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float maxH;

out vec4 vertexColor;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
	vertexColor = vec4(aColor, 1.0);
}