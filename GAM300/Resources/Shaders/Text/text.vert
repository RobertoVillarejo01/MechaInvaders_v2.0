#version 440 core 
 
layout (location = 0) in vec2 aPos; 
layout (location = 1) in vec2 aUVs;
 
out vec2 TextureCoord;
 
layout(location = 0) uniform mat4 MtxStringToModel; 
layout(location = 1) uniform mat4 MtxCharToString; 
layout(location = 4) uniform mat4 MtxWorldToView; 
layout(location = 5) uniform mat4 MtxModelToWorld; 
layout(location = 6) uniform mat4 MtxWorldToProj; 

layout(location = 7) uniform bool is_hBill; 
layout(location = 8) uniform bool is_vBill; 
 
void main() 
{
  // Extract the necessary vectors from the camera mtx
  vec3 CamRightVec = normalize(vec3(MtxWorldToView[0][0], MtxWorldToView[1][0], MtxWorldToView[2][0]));
  vec3 CamUpVec    = normalize(vec3(MtxWorldToView[0][1], MtxWorldToView[1][1], MtxWorldToView[2][1]));
  
  // Get only the part that is supposed to be part of the model rotated (char/string transforms too)
  vec4 on_string = MtxStringToModel * MtxCharToString * vec4(aPos, 1.0f, 1.0f);
  
  // Multiply the respective coordinate by the vectors of the camera or the standar ones, depending
  // on the billboarding for said axis being active
  vec3 h_bill = is_hBill ? CamRightVec : vec3(1,0,0);
  vec3 v_bill = is_vBill ? CamUpVec : vec3(0,1,0);
    
  vec3 Rotated = h_bill * on_string.x + v_bill * on_string.y + vec3(0,0,on_string.z);
 
  // Finally apply world to proj transforms to the rotated model
  gl_Position = MtxWorldToProj * MtxModelToWorld * vec4(Rotated, 1.0);
  TextureCoord = vec2(aUVs.x, 1-aUVs.y);
}