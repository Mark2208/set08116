#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

geometry geom;
effect eff;
target_camera cam;
float theta = 0.0f;
float rho = 0.0f;
vec3 pos(0.0f, 0.0f, 0.0f);
vec3 sizeScale(1.0f, 1.0f, 1.0f);
float s = 1.0f;
float total_time = 0.0f;

bool load_content() {
  // Create cube data - twelve triangles triangles
  // Positions
  vector<vec3> positions{
      // *********************************
      // Add the position data for triangles here, (6 verts per side)
	  // Front
	  vec3(-1.0f,1.0f,0.0f),//FTL
	  vec3(1.0f,1.0f,0.0f),//FTR
	  vec3(1.0f,-1.0f,0.0f),//FBR
	  vec3(1.0f,-1.0f,0.0f),//FBR
	  vec3(-1.0f,-1.0f,0.0f),//FBL
	  vec3(-1.0f,1.0f,0.0f),//FTL


	// Back
	vec3(-1.0f,1.0f,2.0f),//BTL
	vec3(-1.0f,-1.0f,2.0f),//BBL
	vec3(1.0f,-1.0f,2.0f),//BBR
	vec3(1.0f,-1.0f,2.0f),//BBR
	vec3(1.0f,1.0f,2.0f),//BTR
	vec3(-1.0f,1.0f,2.0f),//BTL

	// Right
	vec3(1.0f,1.0f,2.0f),//BTR
	vec3(1.0f,-1.0f,2.0f),//BBR
	vec3(1.0f,-1.0f,0.0f),//FBR
	vec3(1.0f,-1.0f,0.0f),//FBR
	vec3(1.0f,1.0f,0.0f),//FTR
	vec3(1.0f,1.0f,2.0f),//BTR

	// Left
	vec3(-1.0f,-1.0f,2.0f),//BBL
	vec3(-1.0f,1.0f,2.0f),//BTL
	vec3(-1.0f,-1.0f,0.0f),//FBL
	vec3(-1.0f,-1.0f,0.0f),//FBL
	vec3(-1.0f,1.0f,2.0f),//BTL
	vec3(-1.0f,1.0f,0.0f),//FTL

	// Top
	vec3(-1.0f,1.0f,2.0f),//BTL
	vec3(1.0f,1.0f,2.0f),//BTR
	vec3(-1.0f,1.0f,0.0f),//FTL
	vec3(-1.0f,1.0f,0.0f),//FTL
	vec3(1.0f,1.0f,2.0f),//BTR
	vec3(1.0f,1.0f,0.0f),//FTR

	// Bottom
	vec3(-1.0f,-1.0f,2.0f),//BBL
	vec3(1.0f,-1.0f,0.0f),//FBR
	vec3(1.0f,-1.0f,2.0f),//BBR
	vec3(-1.0f,-1.0f,2.0f),//BBL
	vec3(-1.0f,-1.0f,0.0f),//FBL
	vec3(1.0f,-1.0f,0.0f)//FBR


      // *********************************
  };
  // Colours
  vector<vec4> colours;
  for (auto i = 0; i < positions.size(); ++i) {
    colours.push_back(vec4(i % 2, 0.6, 0.0f, 1.0f)); // Notice how I got those Rad colours?
  }
  // Add to the geometry
  geom.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
  geom.add_buffer(colours, BUFFER_INDEXES::COLOUR_BUFFER);

  // Load in shaders
  eff.add_shader("shaders/basic.vert", GL_VERTEX_SHADER);
  eff.add_shader("shaders/basic.frag", GL_FRAGMENT_SHADER);
  // Build effect
  eff.build();

  // Set camera properties
  cam.set_position(vec3(10.0f, 10.0f, 10.0f));
  cam.set_target(vec3(0.0f, 0.0f, 0.0f));
  auto aspect = static_cast<float>(renderer::get_screen_width()) / static_cast<float>(renderer::get_screen_height());
  cam.set_projection(quarter_pi<float>(), aspect, 2.414f, 1000.0f);
  return true;
}

bool update(float delta_time) {
  // *********************************
  // Use keys to update transform values
  // WSAD - movement
  // Arrow Keys - rotation
  // O decrease scale, P increase scale

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_W)) {
		pos += vec3(0.0f, 0.0f, -5.0f) * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_S)) {
		pos += vec3(0.0f, 0.0f, 5.0f) * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_A)) {
		pos += vec3(-5.0f, 0.0f, 0.0f) * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_D)) {
		pos += vec3(5.0f, 0.0f, 0.0f) * delta_time;
	}

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_UP)) {
		theta -= pi<float>() * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_DOWN)) {
		theta += pi<float>() * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_RIGHT)) {
		rho -= pi<float>() * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT)) {
		rho += pi<float>() * delta_time;
	}


	if (glfwGetKey(renderer::get_window(), GLFW_KEY_O)) {

		sizeScale += vec3(0.01f,0.01f,0.01f);
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_P)) {
		sizeScale -= vec3(0.01f, 0.01f, 0.01f);
	}




  // *********************************
  // Update the camera
  cam.update(delta_time);
  return true;
}

bool render() {
  // Bind effect
  renderer::bind(eff);
  
  // *********************************
  // Create transformation matrix
  mat4 n(1.0f);
  mat4 T = translate(n, pos);
  mat4 R = eulerAngleXZ(theta, rho);
  mat4 S = scale(n, sizeScale);
  mat4 M(1.0f);
  M = M * T * R * S;

  // *********************************
  // Create MVP matrix
  auto V = cam.get_view();
  auto P = cam.get_projection();
  auto MVP = P * V * M;
  // Set MVP matrix uniform
  glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
  // Render geometry
  renderer::render(geom);
  return true;
}

void main() {
  // Create application
  app application("20_Transformed_Cube");
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
}