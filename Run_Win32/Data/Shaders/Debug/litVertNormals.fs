#version 420 core

// Uniforms ==============================================
// Constants
uniform vec3 EYE_POSITION;  // camera related

// Scene related
uniform vec4 AMBIENT; // xyz color, w intensity

// lighting
uniform vec3 LIGHT_POSITION; 
uniform vec4 LIGHT_COLOR; // xyz color, w intensity

// surface 
uniform float SPECULAR_AMOUNT; // shininess (0 to 1)
uniform float SPECULAR_POWER; // smoothness (1 to whatever)

// Textures
layout(binding = 0) uniform sampler2D gTexDiffuse;


// Suggest always manually setting bindings - again, consitancy with 
// other rendering APIs and well as you can make assumptions in your
// engine without having to query
//layout(binding = 0) uniform sampler2D gTexDiffuse;

// Attributes ============================================
in vec2 passUV; 
in vec4 passColor; 
in vec3 passWorldPos;   // new
in vec3 passWorldNormal;// new

out vec4 outColor; 

// Entry Point ===========================================
void main( void )
{

   vec3 surface_normal = normalize(passWorldNormal);

   //debug normals
   vec3 normalColor = (surface_normal + vec3(1.0f)) * .5f;
   outColor = vec4(normalColor, 1.f);
}