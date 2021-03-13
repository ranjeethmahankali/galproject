
#version 330 core

out vec4 FragColor;

in vec4 vertexColor;
// uniform vec3 color;

void main()
{
  // FragColor = vec4(color * vertexColor, 1.0);
  FragColor = vertexColor;
}