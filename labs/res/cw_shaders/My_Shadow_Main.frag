#version 440

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

// Spot light being used in the scene
uniform spot_light spot;
// Material of the object being rendered
uniform material mat;
// Position of the eye (camera)
uniform vec3 eye_pos;
// Texture to sample from
uniform sampler2D tex;

// Incoming position
layout(location = 0) in vec3 position;
// Incoming normal
layout(location = 1) in vec3 normal;
// Incoming texture coordinate
layout(location = 2) in vec2 tex_coord;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
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

   // Calculate view direction (light to face)
   vec3 R = spot.direction;

  float RLmaxp = pow(max(dot((-1)*R,L),0.0f),light_power);
  custom_light_col = (spot.light_colour * RLmaxp) / (kc + kld + kqd2);


  // Now use standard phong shading but using calculated light colour and direction
  // - note no ambient

  //Light Direction
  //Defined previously

   // Calculate diffuse component
  float k = max(dot(normal,L),0.0f);
  vec4 diffuse = ((mat.diffuse_reflection)*(custom_light_col))*k;


  // Calculate view direction
  vec3 view_dir = normalize(eye_pos - position);
  // Calculate half vector
   vec3 half_v = normalize(L + view_dir);
   // Calculate specular component
  float s = pow((max(dot(normal, half_v), 0.0f)),mat.shininess);
  vec4 specular =((mat.specular_reflection) * (custom_light_col))*s;


   // Sample texture
  vec4 sampleTex = texture(tex,tex_coord);

  // Calculate primary colour component
   vec4 primary = mat.emissive + diffuse;
   vec4 secondary = specular;

  // Calculate final colour - remember alpha
   primary.w = 1.0f;
   secondary.w = 1.0f;

   colour = sampleTex*primary + secondary;

  // *********************************
}