
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
out vec4 vertNorm;

uniform mat4 mvpMat;
uniform bool pointMode;
uniform bool edgeMode;

void main()
{
  gl_Position = mvpMat * vec4(position, 1.0);
  if (!pointMode && !edgeMode) {
    vertNorm = mvpMat * vec4(normal, 0.0);
  }
  if (pointMode) {
    gl_PointSize = 8.0;
  }
}