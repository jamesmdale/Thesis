#version 420 core
#define MAX_LIGHTS 8

// Uniforms ==============================================
// Constants
uniform vec3 EYE_POSITION;  // camera related
uniform float TIME;

// Scene related
//uniform vec4 AMBIENT; // xyz color, w intensity

// surface
//uniform float SPECULAR_AMOUNT; // shininess (0 to 1)
//uniform float SPECULAR_POWER; // smoothness (1 to whatever)

// per light values
//uniform vec3 LIGHT_POSITION[MAX_LIGHTS];
//uniform vec4 LIGHT_COLOR[MAX_LIGHTS]; //w is intensity

//attenuation
//uniform vec3 ATTENUATION[MAX_LIGHTS];
//uniform vec3 LIGHT_FORWARD[MAX_LIGHTS];
//uniform vec3 LIGHT_DIRECTION_FACTOR[MAX_LIGHTS]; //0 means point light. 1 means directional (also matters for spot)
//uniform float LIGHT_INNER_ANGLE[MAX_LIGHTS];
//uniform float LIGHT_OUTER_ANGLE[MAX_LIGHTS];

struct Light
{
   vec3 position;
   float directionFactor;

   vec4 colorAndIntensity;

   vec3 attenuation;
   float innerAngle;

   vec3 forward;
   float outerAngle;

   mat4 viewProjection;

   vec3 padding;
   float isShadowCasting;
};

//--------------------------------------------------------------------------------------
// CONSTANT BUFFER
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
layout (binding=1, std140) uniform LightBlock
{
   vec4 AMBIENT; // alpha is intensity
   Light LIGHTS[MAX_LIGHTS];
};

//--------------------------------------------------------------------------------------
layout (binding=2, std140) uniform SpecularBlock
{
   float SPECULAR_AMOUNT;
   float SPECULAR_POWER;
   vec2 padding0;
};
// Textures
layout(binding = 0) uniform sampler2D gTexDiffuse;
layout(binding = 1) uniform sampler2D gTexNormal;
layout(binding = 5) uniform sampler2DShadow gTexShadow;


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


float GetShadowFactor( vec3 position, vec3 normal, Light light )
{
   float shadow = light.isShadowCasting;
   if (shadow == 0.0f) {
      return 1.0f;
   }

   // so, we're lit, so we will use the shadow sampler
   float bias_factor = max( dot( light.forward, normal ), 0.0f );
   bias_factor = sqrt(1 - (bias_factor * bias_factor));
   position -= light.forward * bias_factor * .25f;

   vec4 clip_pos = light.viewProjection * vec4(position, 1.0f);
   vec3 ndc_pos = clip_pos.xyz / clip_pos.w;

   // put from -1 to 1 range to 0 to 1 range
   ndc_pos = (ndc_pos + vec3(1)) * .5f;

   // can give this a "little" bias
   // treat every surface as "slightly" closer"
   // returns how many times I'm pass (GL_LESSEQUAL)
   float is_lit = texture( gTexShadow, ndc_pos ).r;
   // float my_depth = ndc_pos.z;

   // use this to feathre shadows near the border
   float min_uv = min( ndc_pos.x, ndc_pos.y );
   float max_uv = max( ndc_pos.x, ndc_pos.y );
   float blend = 1.0f - min( smoothstep(0.0f, .05f, min_uv), smoothstep(1.0, .95, max_uv) );

   // step returns 0 if nearest_depth is less than my_depth, 1 otherwise.
   // if (nearest_depth) is less (closer) than my depth, that is shadow, so 0 -> shaded, 1 implies light
   // float is_lit = step( my_depth, nearest_depth ); //

   // scale by shadow amount
   return mix( light.isShadowCasting * is_lit, 1.0f, blend );
}

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

   // used in final lighting equation to compute
   // final color of output - calculated from the light
   vec3 surface_light = vec3(0.0f); // How much light is hitting the surface (diffuse)
   vec3 reflected_light = vec3(0.0f);  // How much light is reflected back (specular light)

   // first, add in Ambient light (general brightness of the room)
   // to our surface light
   surface_light = AMBIENT.xyz * AMBIENT.w;



   // ... normal and eye_dir calculating as per normal, unique per pixel FOR EACH LIGHT
     for (int lightIndex = 0; lightIndex < MAX_LIGHTS; ++lightIndex)
     {

         vec3 light_dir = LIGHTS[lightIndex].position - passWorldPos;
         float distanceToLight = length(light_dir); // direction to light
         light_dir = normalize(light_dir);

         float f = dot( -light_dir, LIGHTS[lightIndex].forward );

         //get light forward
         vec3 light_forward = normalize(LIGHTS[lightIndex].forward); //normalize just in case

         float shadowFactor = GetShadowFactor(passWorldPos, world_normal, LIGHTS[lightIndex]);

         // figure out how far away angle-wise I am from the forward of the light (useful for spot lights)
         float dot_angle = dot( light_forward, -light_dir );

         float lightIntensity = LIGHTS[lightIndex].colorAndIntensity.w;

         // falloff for spotlights.
         float angle_attenuation = smoothstep( LIGHTS[lightIndex].outerAngle, LIGHTS[lightIndex].innerAngle, dot_angle );
         //angle_attenuation = RangeMapFloat0to1(angle_attenuation, 0.0f, 360.f);
         angle_attenuation = clamp(angle_attenuation, 0.0f, 1.0f);
         lightIntensity = lightIntensity * angle_attenuation;

         // get actual direction light is pointing (spotlights point in their "forward", point lights point everywhere (ie, toward the point))
         light_dir = mix(light_dir, -light_forward, LIGHTS[lightIndex].directionFactor);

         float attenuation = lightIntensity / (LIGHTS[lightIndex].attenuation.x + (LIGHTS[lightIndex].attenuation.y * distanceToLight) + (LIGHTS[lightIndex].attenuation.z * distanceToLight * distanceToLight));

         vec3 eye_dir = normalize(EYE_POSITION - passWorldPos); // direction to the eye

         // Next, for this light, how much of it is hitting the surface
         float dot3 = dot(light_dir, world_normal);

         vec3 light_color = max(dot3, 0.0f) * LIGHTS[lightIndex].colorAndIntensity.xyz * attenuation;
         surface_light += light_color * shadowFactor;

         // Next compute the reflected light
         float spec_factor = 0.0f;

         // get the direction of the light reflected off the surface...
         vec3 reflectionVector = reflect( -light_dir, world_normal );
         // ... and compute how much it hits our eye.
         spec_factor = max( 0, dot( eye_dir, reflectionVector ) );

         // Scale and sharpen the reflection to simulate general
         // shininess and roughness of of the surface
         spec_factor = SPECULAR_AMOUNT * pow( spec_factor, SPECULAR_POWER );

         // Add in the reflected power of this light. (spec)
         reflected_light += light_color * spec_factor * attenuation * shadowFactor;
     }

   // Surface lighting should never blow
   // out your surface.  It is either fully lit or not,
   // so clamp it.
   surface_light = clamp(surface_light, vec3(0), vec3(1));

   // Reflected light on the other hand CAN exceed one (used for bloom)

   // Calculate final color; note, it should not change the alpha... so...
   // Alpha of one for surface light as it is multiplied in,
   // and 0 for specular since it is added in.
   vec4 final_color = vec4(surface_light, 1) * surface_color + vec4(reflected_light, 0);

   // Reclamp so that we're in a valid colour range.  May want to save off overflow
   // if doing bloom.
   final_color = clamp(final_color, vec4(0), vec4(1) );

   // Output the colour
   outColor = final_color;

}
