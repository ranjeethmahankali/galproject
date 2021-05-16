#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 offset;
layout(location = 2) in vec2 texCoords;

out vec2 TexCoords;

uniform mat4 mvpMat;

void main()
{
  gl_Position = (mvpMat * vec4(position, 1.0)) + vec4(offset, 0.0, 0.0);
  TexCoords = texCoords;
}
