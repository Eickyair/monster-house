#version 330 core
out vec4 FragColor;

uniform vec4 lightColor;

void main()
{
    // Bright color for light indicators
    FragColor = vec4(lightColor.rgb * 1.5, 1.0);
}
