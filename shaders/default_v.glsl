#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec4 vertNorm;
out vec4 vertNormWorld;

uniform mat4 mvpMat;
uniform bool orthoMode;
uniform bool pointMode;
uniform bool edgeMode;

void main()
{
  gl_Position = mvpMat * vec4(position, 1.0);
  if (!pointMode && !edgeMode) {
    vertNorm      = normalize(mvpMat * vec4(normal, 0.0));
    vertNormWorld = normalize(vertNorm);
  }
  if (pointMode) {
    gl_PointSize = 10.0;
  }
}