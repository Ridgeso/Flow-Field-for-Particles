#version 330 core

in vec4 Color;

uniform sampler2D tex;

void main()
{
    gl_FragColor = Color;
}