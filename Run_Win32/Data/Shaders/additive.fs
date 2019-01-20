#version 420 core
#define MAX_LIGHTS 8

// Uniforms ==============================================
uniform vec4 TINT;

// Textures
layout(binding = 0) uniform sampler2D gTexDiffuse;

// Attributes ============================================
in vec3 passWorldPos;
in vec2 passUV; 
in vec4 passColor; 

out vec4 outColor; 

// Entry Point ===========================================
void main( void )
{
 
   vec4 texColor = texture(gTexDiffuse, passUV) * TINT;

   texColor *= texColor.a; //mulitply alpha through
 
   outColor = texColor * passColor;
}