#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texCoord;

uniform mat4 ortho;

out vec4 edgeColor;

void main()
{
   edgeColor = color;
   gl_Position = ortho * position;
}