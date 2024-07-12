#version 330 core

uniform mat4 transform;

layout(location=0)in vec3 position;
layout(location=1)in vec3 color;
layout(location=2)in vec2 texCoord;

out vec2 TexCoord;

void main()
{
    gl_Position=transform*vec4(position,1.f);
    TexCoord=vec2(texCoord.x,texCoord.y);
}
