#version 440

// A directional light structure
struct directional_light {
  vec4 ambient_intensity;
  vec4 light_colour;
  vec3 light_dir;
};

// A material structure
struct material {
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};

// Directional light for the scene
uniform directional_light light;
// Material of the object
uniform material mat;
// Position of the camera
uniform vec3 eye_pos;
// Texture
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
  // Calculate ambient component
  vec4 ambient_col = (mat.diffuse_reflection) * (light.ambient_intensity);

  // Calculate diffuse component
  float k = max(dot(normal,light.light_dir),0.0f);
  vec4 diffuse = k * ((mat.diffuse_reflection)*(light.light_colour));


  // Calculate view direction
  vec3 world_pos = vec3(M * vec4(position,1.0f));
  vec3 view_dir = normalize(eye_pos - world_pos);

  // Calculate half vector
   vec3 half_v = normalize(light.light_dir + view_dir);

  // Calculate specular component
  float s = pow((max(dot(normalTransformed, half_v), 0.0f)),mat.shininess);
  vec4 specular =((mat.specular_reflection) * (light.light_colour))*s;

  // Sample texture
  vec4 sampleTex = texture(tex,tex_coord);

  // Calculate primary colour component
   vec4 primary = mat.emissive + ambient_col + diffuse;
   vec4 secondary = specular;

  // Calculate final colour - remember alpha
   primary.w = 1.0f;
   secondary.w = 1.0f;

   colour = sampleTex*primary + secondary;
  // *********************************
}