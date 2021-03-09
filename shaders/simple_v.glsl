
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
out vec3 vertexColor;

uniform mat4 mView;
uniform mat4 mProj;

void main()
{
  mat4 mvp    = mProj * mView;
  gl_Position = mvp * vec4(position, 1.0);

  float fdot =
    clamp(dot(normalize(mvp * vec4(normal, 0.0)), vec4(0.0, 0.0, -1.0, 0.0)), 0.0, 1.0);

  fdot        = 0.1 * (1.0 - fdot) + 1.0 * fdot;
  vertexColor = vec3(1.0, 1.0, 1.0) * fdot;
}