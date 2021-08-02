#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 offset;
layout(location = 2) in vec2 texCoords;

out vec2 TexCoords;

uniform mat4 mvpMat;
uniform bool orthoMode;

void main()
{
  vec4 pos3d = mvpMat * vec4(position, 1.0);
  vec4 shift = vec4(offset + vec2(0.01, 0.01), 0.0, 0.0);
  if (!orthoMode) {
    // Multiplying with the depth (z) cancels out the perspective scaling.
    shift *= pos3d.z;
  }
  gl_Position = pos3d + shift;
  TexCoords   = texCoords;
}
