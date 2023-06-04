#version 440 core

in vec4 FragPos;

layout(location = 1) uniform vec3  lightPos;
layout(location = 2) uniform float far_plane;

void main()
{
    // Get distance between fragment and light source
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // Map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;
    
    // Write this as modified depth (it will make 
    // computations easier when rendering the shadows)
    //gl_FragDepth = 0.0;
    gl_FragDepth = lightDistance;
}