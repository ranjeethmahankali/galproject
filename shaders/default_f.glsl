
#version 330 core

out vec4 FragColor;

in vec4 vertNorm;

uniform bool  edgeMode;
uniform bool  pointMode;
uniform float shadingFactor;
uniform vec4  faceColor;
uniform vec4  edgeColor;
uniform vec4  pointColor;

void main()
{
  if (edgeMode) {
    FragColor = edgeColor;
  }
  else if (pointMode) {
    FragColor = pointColor;
  }
  else {
    float fdot = abs(dot(normalize(vertNorm), vec4(0.0, 0.0, -1.0, 0.0)));

    fdot = clamp(0.1 * (1.0 - fdot) + 1.0 * fdot, 0.0, 1.0);
    fdot = fdot * shadingFactor + 1.0f * (1.0f - shadingFactor);

    FragColor.x = faceColor.x * fdot;
    FragColor.y = faceColor.y * fdot;
    FragColor.z = faceColor.z * fdot;
    FragColor.a = faceColor.a;
  }
}