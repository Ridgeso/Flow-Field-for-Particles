#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 angle;

uniform mat4 ortho;

out vec4 Color;

void main()
{
   Color = color;

   gl_Position = ortho * position;
}