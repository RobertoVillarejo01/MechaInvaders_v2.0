#version 440 core

// Rendering the scene into 2 different targets so we can have the bloom effect
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BloomColor;

layout(location = 1) uniform vec4 uniform_color;
layout(location = 2) uniform bool im_emitter;

void main()
{
  FragColor = uniform_color;
  if (FragColor.a <= 0.0)
    discard;

  if (im_emitter) {
    FragColor = vec4(FragColor.rgb + 20.0, FragColor.a);
  }

  // Check if the "brightness" of the fragment surpasses a certain threshold
  float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
  if(brightness > 1.0)
    BloomColor = vec4(FragColor.rgb, 1.0);
  else
    BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
}
