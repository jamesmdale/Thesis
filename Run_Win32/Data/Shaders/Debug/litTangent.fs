#version 420 core

// Textures
layout(binding = 0) uniform sampler2D gTexDiffuse;


// Suggest always manually setting bindings - again, consitancy with 
// other rendering APIs and well as you can make assumptions in your
// engine without having to query
//layout(binding = 0) uniform sampler2D gTexDiffuse;

// Attributes ============================================
in vec3 passWorldTangent;

out vec4 outColor; 


// Entry Point ===========================================
void main( void )
{
  // Interpolation is linear, so normals become not normal
   // over a surface, so renormalize it. 
   vec3 world_vtan = normalize(passWorldTangent);

   //debug normals
   vec3 tangentColor = (world_vtan + vec3(1.0f)) * .5f;
   outColor = vec4(tangentColor, 1.f);
}