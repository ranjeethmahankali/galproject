
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
out vec3 vertexColor;

uniform mat4 mView;
uniform mat4 mProj;

void main()
{
  gl_Position = mProj * mView * vec4(position, 1.0);
  float fdot =
    dot(normalize(mProj * mView * vec4(normal, 0.0)), vec4(0.0, 0.0, -1.0, 0.0));
  // float fdot  = 0.5;
  vertexColor = vec3(1.0, 1.0, 1.0) * fdot;
}