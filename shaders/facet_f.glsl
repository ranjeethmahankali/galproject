
#version 330 core

out vec4 FragColor;

in vec3 vertexColor;
in vec3 bary;

void main()
{
  // FragColor = vec4(color * vertexColor, 1.0);
  // if (bary.x < 0.01 || bary.y < 0.01 || bary.z < 0.01) {
  //   FragColor = vec4(1.0, 0.0, 0.0, 1.0);
  // }
  // else {

  // }
  FragColor = vec4(vertexColor, 1.0);
}