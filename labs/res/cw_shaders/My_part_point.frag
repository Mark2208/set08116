#version 440
// Point light information
#ifndef POINT_LIGHT
#define POINT_LIGHT
struct point_light
{
	vec4 light_colour;
	vec3 position;
	float constant;
	float linear;
	float quadratic;
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

// Point light calculation
vec4 calculate_point(in point_light point, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_colour)
{
	// *********************************
  // Get distance between point light and vertex
  float d = distance(point.position , position);

  // Calculate attenuation factor
  float kc = point.constant;
  float kld = point.linear * d;
  float kqd2 = (point.quadratic) * pow(d,2); 

  // Calculate light colour
  vec4 custom_light_col = point.light_colour;
  custom_light_col = point.light_colour / (kc + kld + kqd2);

  // Calculate light dir
   vec3 L = normalize(point.position - position);

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

   vec4 point_colour = sampleTex*primary + secondary;

  // *********************************
  return point_colour;
}