#version 330

uniform vec3 FragColor;

out vec4 color;

void main()
{
    color = vec4(FragColor, 1.0f);
}