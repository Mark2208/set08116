#version 440

// Point light information
struct point_light {
  vec4 light_colour;
  vec3 position;
  float constant;
  float linear;
  float quadratic;
};

// Spot light data
struct spot_light {
  vec4 light_colour;
  vec3 position;
  vec3 direction;
  float constant;
  float linear;
  float quadratic;
  float power;
};

// Material data
struct material {
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};


// Point lights being used in the scene
uniform point_light points[4];
// Spot lights being used in the scene
uniform spot_light spots[5];
// Material of the object being rendered
uniform material mat;
// Position of the eye (camera)
uniform vec3 eye_pos;
// Texture to sample from
uniform sampler2D tex;

uniform sampler2D normal_map;

uniform sampler2D shadow_map;

uniform float normalMapEnabled;

uniform vec4 ambientIntensity;


uniform vec4 fog_colour;
uniform float fog_start;
uniform float fog_end;
uniform float fog_density;
uniform int fog_type;

// Incoming position
layout(location = 0) in vec3 position;
// Incoming normal
layout(location = 1) in vec3 normal;
// Incoming texture coordinate
layout(location = 2) in vec2 tex_coord;

// Incoming tangent
layout(location = 3) in vec3 tangent;
// Incoming binormal
layout(location = 4) in vec3 binormal;

layout(location = 5) in vec4 light_space_pos;

layout(location = 6) in vec4 CS_position;

// Outgoing colour
layout(location = 0) out vec4 colour;



vec3 calc_normal(in vec3 normal, in vec3 tangent, in vec3 binormal, in sampler2D normal_map, in vec2 tex_coord);


// Point light calculation
vec4 calculate_point(in point_light point, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir,
                     in vec4 tex_colour);

// Spot light calculation
vec4 calculate_spot(in spot_light spot, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_colour);

float calculate_shadow(in sampler2D shadow_map, in vec4 light_space_pos);

float calculate_fog(in float fog_coord, in vec4 fog_colour, in float fog_start, in float fog_end, in float fog_density, in int fog_type);

void main() {

  colour = vec4(0.0, 0.0, 0.0, 1.0);
  // *********************************

  // Calculate shade factor
  float shade_factor = calculate_shadow(shadow_map,light_space_pos);

  // Calculate view direction
   vec3 view_dir = normalize(eye_pos - position);

  // Sample texture
   vec4 sampleTex = texture(tex,tex_coord);

   vec3 mappedNormal = normal;
   //0 = True
   if (normalMapEnabled == 0)
   {
	  mappedNormal = calc_normal(normal,  tangent,  binormal, normal_map, tex_coord);
   }
  
   vec4 point_colour = vec4(0.0,0.0,0.0,1.0);
   vec4 spot_colour = vec4(0.0,0.0,0.0,1.0);
   
 
  for (int i = 0 ; i < 4 ; ++i)
  {
	  point_colour += calculate_point(points[i], mat, position , mappedNormal , view_dir , sampleTex);
  }

  for (int i = 0 ; i < 4 ; ++i)
  {
	spot_colour += calculate_spot(spots[i], mat, position , mappedNormal , view_dir ,sampleTex);
  }


   float fog_coord = abs(CS_position.z / CS_position.w);
   float fog_factor = calculate_fog( fog_coord, fog_colour, fog_start, fog_end, fog_density, fog_type);

    
   colour = (ambientIntensity + point_colour + spot_colour * shade_factor);
   colour = mix(colour,fog_colour,fog_factor);

 
  // *********************************
}