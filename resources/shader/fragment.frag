#version 330 core

uniform sampler2D ourTexture;

in vec2 fragTexCoord;

out vec4 outColor;

void main()
{
    outColor=texture(ourTexture,fragTexCoord);
}
