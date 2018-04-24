#version 440
// Spot light data
#ifndef SPOT_LIGHT
#define SPOT_LIGHT
struct spot_light
{
	vec4 light_colour;
	vec3 position;
	vec3 direction;
	float constant;
	float linear;
	float quadratic;
	float power;
};
#endif

// Material data
#ifndef MATERIAL
#define MATERIAL
struct material
{
	vec4 emissive;
	vec4 diffuse_reflection;
	vec4 specular_reflection;
	float shininess;
};
#endif

// Spot light calculation
vec4 calculate_spot(in spot_light spot, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_colour)
{
		// *********************************
  // Calculate direction to the light
  vec3 L = normalize(spot.position - position);

  // Calculate distance to light
   float d = distance(spot.position , position);

  // Calculate attenuation value
  float kc = spot.constant;
  float kld = spot.linear * d;
  float kqd2 = (spot.quadratic) * pow(d,2); 

  // Calculate spot light intensity
  float light_power = spot.power;

  // Calculate light colour
  vec4 custom_light_col = spot.light_colour;

  vec3 R = spot.direction;
  float RLmaxp = pow(max(dot((-1)*R,L),0.0f),light_power);
  custom_light_col = (spot.light_colour * RLmaxp) / (kc + kld + kqd2);

  // Now use standard phong shading but using calculated light colour and direction
  // - note no ambient

   // Calculate diffuse component
  float k = max(dot(normal,L),0.0f);
  vec4 diffuse = ((mat.diffuse_reflection)*(custom_light_col))*k;



  // Calculate half vector
   vec3 half_v = normalize(L + view_dir);
   // Calculate specular component
  float s = pow((max(dot(normal, half_v), 0.0f)),mat.shininess);
  vec4 specular =((mat.specular_reflection) * (custom_light_col))*s;


   // Sample texture
  vec4 sampleTex = tex_colour;

  // Calculate primary colour component
   vec4 primary = mat.emissive + diffuse;
   vec4 secondary = specular;

  // Calculate final colour - remember alpha
   primary.w = 1.0f;
   secondary.w = 1.0f;

   vec4 colour = sampleTex*primary + secondary;




  // *********************************
  return colour;
}