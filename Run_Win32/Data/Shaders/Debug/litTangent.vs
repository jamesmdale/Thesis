#version 420 core

// Uniforms ==============================================
uniform mat4 MODEL;
uniform mat4 VIEW;
uniform mat4 PROJECTION; 

// Attributes ============================================
// Inputs
in vec3 POSITION;
in vec4 TANGENT;

// Outputs
out vec3 passWorldTangent;


// Entry Point ===========================================
void main( void )
{
   vec4 local_pos = vec4( POSITION, 1.0f );  

   vec4 world_pos = MODEL * local_pos; 
   vec4 camera_pos = VIEW * world_pos; 
   vec4 clip_pos =  PROJECTION * camera_pos; 

   passWorldTangent = normalize((MODEL * vec4(TANGENT.xyz, 0.0f)).xyz); 

   gl_Position = clip_pos; // we pass out a clip coordinate
}