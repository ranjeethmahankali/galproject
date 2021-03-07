
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
out vec3 vertexColor;

uniform mat4 mView;
uniform mat4 mProj;

const vec3 frontColor = vec3(1.0, 1.0, 1.0);
const vec3 backColor  = vec3(0.5725, 0.0, 0.0);

void normalizeFDot(inout float fdot)
{
  fdot = 0.2 * (1.0 - fdot) + 0.8 * fdot;
}

void main()
{
  gl_Position = mProj * mView * vec4(position, 1.0);

  float fdot =
    dot(normalize(mProj * mView * vec4(normal, 0.0)), vec4(0.0, 0.0, -1.0, 0.0));

  if (fdot < 0.0) {
    fdot = -fdot;
    normalizeFDot(fdot);
    vertexColor = backColor * fdot;
  }
  else {
    normalizeFDot(fdot);
    vertexColor = frontColor * fdot;
  }
}