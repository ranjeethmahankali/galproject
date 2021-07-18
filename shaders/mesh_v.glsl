#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

out vec4 vertNorm;
out vec4 vertNormWorld;
out vec3 vertColor;

uniform mat4 mvpMat;
uniform bool orthoMode;
uniform bool pointMode;
uniform bool edgeMode;

void main()
{
  gl_Position = mvpMat * vec4(position, 1.0);
  if (!edgeMode) {
    vertNorm      = normalize(mvpMat * vec4(normal, 0.0));
    vertNormWorld = normalize(vertNorm);
    vertColor     = color;
  }
  if (pointMode) {
    gl_PointSize = 10.0;
  }
}