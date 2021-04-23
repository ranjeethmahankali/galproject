
#version 330 core

out vec4 FragColor;

in vec4 vertNorm;
in vec4 vertNormWorld;

uniform bool  edgeMode;
uniform bool  pointMode;
uniform bool  orthoMode;
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
    // lighting w.r.t eye pos.
    float fdot = abs(dot(vertNorm, vec4(0.0, 0.0, -1.0, 0.0)));
    // Lighting w.r.t. global normal.
    float fdot2 = abs(dot(vertNormWorld, vec4(1.0, 0.0, 0.0, 0.0)));
    fdot2       = 0.2 * fdot2 + (1.0 - fdot2) * 0.8;  // reduce contrast of fdot2.
    fdot        = 0.5 * (fdot + fdot2);
    // reduce contrast and clamp
    fdot = clamp(0.1 * (1.0 - fdot) + 1.5 * fdot, 0.0, 1.0);
    fdot = fdot * shadingFactor + 1.0 * (1.0 - shadingFactor);

    if (!orthoMode) {
      fdot *= 0.7;
    }

    FragColor.x = faceColor.x * fdot;
    FragColor.y = faceColor.y * fdot;
    FragColor.z = faceColor.z * fdot;
    FragColor.a = faceColor.a;
  }
}