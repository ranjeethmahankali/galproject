
#version 330 core

layout(location = 0) in vec3 position;
out vec3 vertexColor;

uniform mat4 mView;
uniform mat4 mProj;

void main()
{
  gl_Position = mProj * mView * vec4(position, 1.0);
  vertexColor = vec3(1.0, 1.0, 1.0);
}