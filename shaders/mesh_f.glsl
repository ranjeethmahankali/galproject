#version 330 core

out vec4 FragColor;

in vec4 vertNorm;
in vec4 vertNormWorld;
in vec3 vertColor;

uniform bool edgeMode;
uniform vec4 edgeColor;
uniform bool orthoMode;

void main()
{
  if (edgeMode) {
    FragColor = edgeColor;
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

    if (!orthoMode) {
      fdot *= 0.7;
    }

    FragColor.x = vertColor.x * fdot;
    FragColor.y = vertColor.y * fdot;
    FragColor.z = vertColor.z * fdot;
    FragColor.a = 1.0;
  }
}