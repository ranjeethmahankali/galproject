
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 bary;

out vec3 vertexColor;
out vec3 vBary;

uniform mat4 mvpMat;

void main()
{
  gl_Position = mvpMat * vec4(position, 1.0);

  float fdot = clamp(
    dot(normalize(mvpMat * vec4(normal, 0.0)), vec4(0.0, 0.0, -1.0, 0.0)), 0.0, 1.0);

  fdot        = 0.25 * (1.0 - fdot) + 1.0 * fdot;
  vertexColor = vec3(1.0, 1.0, 1.0) * fdot;
  vBary       = bary;
}