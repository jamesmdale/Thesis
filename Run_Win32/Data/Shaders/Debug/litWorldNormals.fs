#version 420 core
#define MAX_LIGHTS 8

// Uniforms ==============================================
// Constants
uniform vec3 EYE_POSITION;  // camera related

// Scene related
uniform vec4 AMBIENT; // xyz color, w intensity

// surface 
uniform float SPECULAR_AMOUNT; // shininess (0 to 1)
uniform float SPECULAR_POWER; // smoothness (1 to whatever)

// per light values
uniform vec3 LIGHT_POSITION[MAX_LIGHTS]; 
uniform vec4 LIGHT_COLOR[MAX_LIGHTS]; //w is intensity

//attenuation
uniform vec3 ATTENUATION[MAX_LIGHTS];

// Textures
layout(binding = 0) uniform sampler2D gTexDiffuse;
layout(binding = 1) uniform sampler2D gTexNormal;


// Suggest always manually setting bindings - again, consitancy with 
// other rendering APIs and well as you can make assumptions in your
// engine without having to query
//layout(binding = 0) uniform sampler2D gTexDiffuse;

// Attributes ============================================
in vec3 passWorldPos;
in vec2 passUV; 
in vec4 passColor; 
in vec3 passWorldNormal;
in vec3 passWorldTangent;
in vec3 passWorldBitangent;

out vec4 outColor; 

// Entry Point ===========================================
void main( void )
{
   // Color of this surface (defuse)
   vec4 surface_color = texture( gTexDiffuse, passUV ) * passColor;

   //get normalcolor
   vec3 normal_color  = texture(gTexNormal, passUV).xyz;

   //get emissive color
   //TODO: EMISSIVE

   // Interpolation is linear, so normals become not normal
   // over a surface, so renormalize it. 
   vec3 world_vnormal = normalize(passWorldNormal);
   vec3 world_vtan = normalize(passWorldTangent);
   vec3 world_vbitan = normalize(passWorldBitangent);

   //SURFACE TO WORLD MATRIX
   mat3 surface_to_world_matrix = mat3(world_vtan, world_vbitan, world_vnormal);

   vec3 surface_normal = normalize(normal_color * vec3(2.0f, 2.0f, 1.0f) + vec3(-1.0f, -1.0f, 0.0f));
   vec3 world_normal = surface_to_world_matrix * surface_normal; //tbn

   vec3 worldNormalColor = (world_normal + vec3(1.0f)) * .5f;
   outColor = vec4(worldNormalColor, 1.f);
}