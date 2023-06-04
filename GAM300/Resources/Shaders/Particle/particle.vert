#version 440 core 
 
layout (location = 0) in vec3 aPos; 
layout (location = 2) in vec2 aUVs; 
layout (location = 5) in vec3 offPos; 
layout (location = 6) in vec3 offScale; 
layout (location = 7) in vec4 offColor; 
 
out vec2 TextureCoord; 
out vec4 VtxColor; 
 
layout(location = 0) uniform mat4 MtxWorldToProj; 
layout(location = 2) uniform mat4 MtxWorldToView; 
 
void main() 
{
  vec3 CamRightVec = normalize(vec3(MtxWorldToView[0][0], MtxWorldToView[1][0], MtxWorldToView[2][0]));
  vec3 CamUpVec    = normalize(vec3(MtxWorldToView[0][1], MtxWorldToView[1][1], MtxWorldToView[2][1]));

  vec3 VertexPosition = offPos + CamRightVec * aPos.x * offScale.x + CamUpVec * aPos.y * offScale.y;
  gl_Position = MtxWorldToProj * vec4(VertexPosition, 1.0f);
  
  VtxColor = offColor; 
  TextureCoord = aUVs; 
}