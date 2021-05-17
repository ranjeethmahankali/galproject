#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D text;
uniform vec3      textColor;

void main()
{
  FragColor = vec4(textColor, texture(text, TexCoords).r);
  // FragColor = vec4(1.0, 1.0, 0.0, 1.0);
  //   FragColor = vec4(texture(text, TexCoords).r, 1.0, 0.0, 1.0);
  //   FragColor = vec4(TexCoords.x, TexCoords.y, 0.0, 1.0);
}
