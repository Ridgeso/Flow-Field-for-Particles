#version 330 core

in vec4 Color;
in vec2 texPos;

uniform sampler2D tex;

void main()
{
    vec4 texPixel = texture(tex, texPos);
    float alpha = step(0.5, texPixel.r);
    gl_FragColor = Color;
    gl_FragColor.a *= alpha;
}