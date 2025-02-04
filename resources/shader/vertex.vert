#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

layout(location = 0)in vec3 position;
layout(location = 1)in vec3 color;
layout(location = 2)in vec2 texCoord;

out vec2 fragTexCoord;

void main()
{
    gl_Position = proj * view * model * vec4(position, 1.f);
    fragTexCoord = vec2(texCoord.x, 1.0 - texCoord.y);
}
