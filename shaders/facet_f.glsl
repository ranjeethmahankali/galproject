
#version 330 core

out vec4 FragColor;

in vec3 vertexColor;
in vec3 vBary;

const float baryLimit = 0.005;

void main()
{
  // FragColor = vec4(color * vertexColor, 1.0);
  if (vBary.x < baryLimit || vBary.y < baryLimit || vBary.z < baryLimit) {
    FragColor = vec4(0.6, 0.0, 0.0, 1.0);
  }
  else {
    FragColor = vec4(vertexColor, 1.0);
  }
}