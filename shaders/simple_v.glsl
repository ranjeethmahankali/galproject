
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
out vec4 vertexColor;

uniform mat4 mvpMat;
uniform bool edgeMode;

uniform vec4 faceColor;
uniform vec4 edgeColor;

void main()
{
  gl_Position = mvpMat * vec4(position, 1.0);
  if (edgeMode) {
    vertexColor = edgeColor;
  }
  else {
    float fdot = clamp(
      dot(normalize(mvpMat * vec4(normal, 0.0)), vec4(0.0, 0.0, -1.0, 0.0)), 0.0, 1.0);

    fdot        = 0.1 * (1.0 - fdot) + 1.0 * fdot;
    vertexColor = faceColor * fdot;
    vertexColor.a = 1.0;
  }
}